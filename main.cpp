/*
    Vidiot id a VIDeo Input Output Transformer With a Touch of Functionality
    Copyright (C) 2016  Delicode Ltd

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "VideoView.h"
#include "VideoThread.h"
#include "FboRenderer.h"
#include "FeedOutput.h"
#include "StdInThread.h"
#include "VidiotApplication.h"
#include "predefines.h"

#ifdef __APPLE__
    #include <ApplicationServices/ApplicationServices.h>
    #include "MacWindowInitializer.h"
#endif

QQuickView *view = NULL;

QStringList formatList()
{
    QStringList formatlist;

    formatlist.append("MPEG-4 (mp4)");
    formatlist.append("Matroska (mkv)");
    formatlist.append("Flash Video (flv)");
    formatlist.append("QuickTime (mov)");

    return formatlist;
}

QStringList qualityList()
{
    QStringList qualitylist;

    qualitylist.append("High");
    qualitylist.append("Medium");
    qualitylist.append("Low");
    qualitylist.append("Stream");

    return qualitylist;
}

QVector<QtAV::VideoDecoderId> decoderIds()
{
    QVector<QtAV::VideoDecoderId> ids;

    QStringList valid_names;

    QSettings settings(QApplication::applicationDirPath() + "/quality.ini", QSettings::IniFormat);

#ifdef _WIN32
    valid_names << "CUDA";
    valid_names << "DXVA";
    valid_names << "D3D11";
    settings.beginGroup("decoders_windows");
#else
    valid_names << "VDA";
    valid_names << "VTBox";
    settings.beginGroup("decoders_osx");
#endif
    valid_names << "FFmpeg";

    for(int i=0; i<10; i++) {
        QString name = settings.value(QString::number(i)).toString();
        if(valid_names.contains(name))
        {
            ids << QtAV::VideoDecoder::id(name.toLatin1().constData());
            qDebug() << "added decoder id: " << name;
        }
    }

    return ids;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#ifdef __APPLE__
    // On OS X, apparently needs to be done before QApplication exists
    // For some reason this can cause shared OpenGL context to not work otherwise
    // Does not happen with the QMLPlayer example though...
    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    VidiotApplication a(argc, argv);

    QCoreApplication::setOrganizationName("Delicode");
    QCoreApplication::setOrganizationDomain("delicode.com");
    QCoreApplication::setApplicationName("Vidiot");

    qmlRegisterType<VideoView>("videoview", 1, 0, "VideoView");
    qRegisterMetaType<RecorderFrame>("RecorderFrame");

    QString input;
    QString output;
    QString quality = "Medium";

    if(argc > 1) {
        if(QString(argv[1]) == "extract_thumbnail") {
            if(argc != 5)
                return 0;

            int w = atoi(argv[2]);
            int h = atoi(argv[3]);

            if(w <= 0 || w > 1000 || h <= 0 || h > 1000)
                return 0;

            VideoPlayer p(argv[4]);
            p.seeking = true;
            p.next_seek_time_s = p.duration/3.f;
            p.play();
            if(p.width > 0 && p.height > 0 && p.data) {
                QImage img(p.width, p.height, QImage::Format_RGB888);
                memcpy(img.bits(), p.data, p.width*p.height*3*sizeof(unsigned char));

                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                img.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation).save(&buffer, "JPG");
                buffer.close();

                std::cout << ":extract_thumbnail:length:" << p.duration << ":thumbnail:" << ba.toBase64().data() << std::endl;
            }
            return 0;
        }
        else if(QString(argv[1]) == "list_devices") {
            QStringList devices = CaptureProcessor::listDevices();
            for(int i=0; i<devices.length(); i++)
                std::cout << devices.at(i).toLocal8Bit().constData() << std::endl;

            return 0;
        }
        else if(QString(argv[1]) == "play") {
            if(argc == 4) {
                input = argv[2];
                output = argv[3];
            }
            else
                return 0;
        }
        else if(QString(argv[1]) == "record") {
            if(argc > 3) {
                input = argv[2];
                output = argv[3];
                if(argc > 4)
                    quality = argv[4];
            }
        }
    }

    QtAV::AVPlayer plr;
    QtAV::AVPlayer *player = NULL;

    QtAV::FboRenderer fr;
    fr.setObjectName("qtav_fborenderer");

    if(output.startsWith("feed:///"))
        FeedOutput::instance().setName(output);

    VideoRecorder::list_codecs(true);

    CaptureThread thread;
    RecordThread rthread;
    StdInThread inthread;

    QObject::connect(&thread.processor, SIGNAL(newRecorderFrame(RecorderFrame)), &rthread.controller, SLOT(recorderFrame(RecorderFrame)));
    QObject::connect(&thread.processor, SIGNAL(stopRecording()), &rthread.controller, SLOT(stop()));

    if(input.isEmpty())
    {
#ifdef __APPLE__
        // The MyInfo.plist file sets LSBackgroundOnly to 1 to not show Dock icon by default
        // This is to hide the icon for when Vidiöt is launched via other software (ie. Z Vector)
        // Since this branch starts Vidiöt with a GUI we want the dock icon to appear
        ProcessSerialNumber psn;
        GetCurrentProcess(&psn);
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        MacWindowInitializer::setApplicationAsUIElement();
        MacWindowInitializer::setFront();

        QCoreApplication::processEvents();
#endif

        view = new QQuickView();
        view->setTitle("Vidiöt");

        view->setIcon(QIcon(":/resources/vidiot_icon.png"));

        thread.processor.setMainView(view);
        view->rootContext()->setContextProperty("formatlist", formatList());
        view->rootContext()->setContextProperty("qualitylist", qualityList());

        QSettings settings;
        view->rootContext()->setContextProperty("initial_format", settings.value("format", "MPEG-4 (mp4)").toString());
        view->rootContext()->setContextProperty("initial_quality", settings.value("quality", "Medium").toString());

        view->rootContext()->setContextProperty("dpi", qApp->primaryScreen()->logicalDotsPerInch()/96.f);

        view->setSource(QUrl("qrc:/qml/main.qml"));
        view->setObjectName("vidiot_qml");

        QObject *object = view->rootObject();

        player = object->findChild<QtAV::AVPlayer*>();
        player->setObjectName("qtav_player");

        VideoView *videoview = object->findChild<VideoView*>();
        QObject::connect(&thread.processor, SIGNAL(sourcesListUpdate(QStringList)), videoview, SLOT(receiveSourcesList(QStringList)));
        QObject::connect(&thread.processor, SIGNAL(resolutionsChanged(QStringList)), videoview, SLOT(receiveResolutionList(QStringList)));
        videoview->setMainView(view);

#ifdef WIN32
        QObject::connect(&fr, SIGNAL(sendFeed(unsigned int, int, int)), &FeedOutput::instance(), SLOT(sendTexture(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeed(uint,int,int,bool)), &FeedOutput::instance(), SLOT(sendTexture(uint,int,int,bool)));
#endif
#ifdef __APPLE__
        // On OS X syphon refuses to transfer textures, but is fine with FBOs
        // Probably something about using rectangle textures instead of square textures
        QObject::connect(&fr, SIGNAL(sendFeedFbo(unsigned int, int, int)), &FeedOutput::instance(), SLOT(sendFBO(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeedFbo(uint,int,int,bool)), &FeedOutput::instance(), SLOT(sendFBO(uint,int,int,bool)));
#endif

        QObject::connect(videoview, SIGNAL(setResolution(QString)), &FeedOutput::instance(), SLOT(setResolution(QString)));
        QObject::connect(&fr, SIGNAL(sendFeed(unsigned int, int, int)), videoview, SLOT(draw(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeed(uint,int,int,bool)), videoview, SLOT(draw(uint,int,int,bool)));
        QObject::connect(&thread.processor, SIGNAL(error(QString)), videoview, SLOT(receiveError(QString)));

        QObject::connect(&fr, SIGNAL(sendResolution(int,int)), &thread.processor, SLOT(updateResolutionList(int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendResolution(int,int)), &thread.processor, SLOT(updateResolutionList(int,int)));

        QObject::connect(videoview, SIGNAL(setSource(QString)), &thread.processor, SLOT(setSource(QString)));
        QObject::connect(videoview, SIGNAL(setResolution(QString)), &thread.processor, SLOT(setResolution(QString)));
        QObject::connect(videoview, SIGNAL(action(QString)), &thread.processor, SLOT(action(QString)));
        QObject::connect(videoview, SIGNAL(updateSources()), &thread.processor, SLOT(updateSourceList()));

        QObject::connect(videoview, SIGNAL(setFormat(QString)), &rthread.controller, SLOT(setFormat(QString)));
        QObject::connect(videoview, SIGNAL(setQuality(QString)), &rthread.controller, SLOT(setQuality(QString)));

        QObject::connect(videoview, SIGNAL(record(bool)), &thread.processor, SLOT(record(bool)));

        QObject::connect(videoview, SIGNAL(setResolution(QString)), &fr, SLOT(setResolution(QString)));

        QObject::connect(&thread.processor, SIGNAL(updateFPS(float)), videoview, SLOT(setFps(float)));
        QObject::connect(&thread.processor, SIGNAL(updateSleeptime(float)), videoview, SLOT(setSleeptime(float)));
        QObject::connect(&thread.processor, SIGNAL(updateRecordtime(float)), videoview, SLOT(setRecordtime(float)));

        QObject::connect(&fr, SIGNAL(updateFPS(float)), videoview, SLOT(setFps(float)));

        QObject::connect(videoview, SIGNAL(showCameraProperties()), &thread.processor, SLOT(cameraPropertiesRequested()));
    }
    else if(input.startsWith("dshow:///")) {
#ifdef WIN32
        QObject::connect(&fr, SIGNAL(sendFeed(unsigned int, int, int)), &FeedOutput::instance(), SLOT(sendTexture(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeed(uint,int,int,bool)), &FeedOutput::instance(), SLOT(sendTexture(uint,int,int,bool)));
#endif
#ifdef __APPLE__
        // On OS X syphon refuses to transfer textures, but is fine with FBOs
        // Probably something about using rectangle textures instead of square textures
        QObject::connect(&fr, SIGNAL(sendFeedFbo(unsigned int, int, int)), &FeedOutput::instance(), SLOT(sendFBO(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeedFbo(uint,int,int,bool)), &FeedOutput::instance(), SLOT(sendFBO(uint,int,int,bool)));
#endif

        thread.processor.setSource(input);
        thread.processor.setPingRequired(true);

        QObject::connect(&inthread.listener, SIGNAL(ping()), &thread.processor, SLOT(ping()));
        QObject::connect(&inthread.listener, SIGNAL(stop()), &thread.processor, SLOT(stop()));

        QObject::connect(&inthread.listener, SIGNAL(showCameraProperties()), &thread.processor, SLOT(cameraPropertiesRequested()));

        inthread.start();
    }
    else if(input.startsWith("feed:///")) {
        thread.processor.setSource(input);
        thread.processor.setPingRequired(true);

        QObject::connect(&inthread.listener, SIGNAL(ping()), &thread.processor, SLOT(ping()));
        QObject::connect(&inthread.listener, SIGNAL(stop()), &thread.processor, SLOT(stop()));
        inthread.start();

        if(output.startsWith("file:///"))
            output.remove("file:///");
        rthread.controller.setFilename(output);
        rthread.controller.setQuality(quality);
    }
    else {
#ifdef WIN32
        QObject::connect(&fr, SIGNAL(sendFeed(unsigned int, int, int)), &FeedOutput::instance(), SLOT(sendTexture(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeed(uint,int,int,bool)), &FeedOutput::instance(), SLOT(sendTexture(uint,int,int,bool)));
#endif
#ifdef __APPLE__
        // On OS X syphon refuses to transfer textures, but is fine with FBOs
        // Probably something about using rectangle textures instead of square textures
        QObject::connect(&fr, SIGNAL(sendFeedFbo(unsigned int, int, int)), &FeedOutput::instance(), SLOT(sendFBO(uint,int,int)));
        QObject::connect(&thread.processor, SIGNAL(sendFeedFbo(uint,int,int,bool)), &FeedOutput::instance(), SLOT(sendFBO(uint,int,int,bool)));
#endif

        player = &plr;

        thread.processor.setPingRequired(true);

        QObject::connect(&inthread.listener, SIGNAL(pause(bool)), player, SLOT(pause(bool)));
        QObject::connect(&inthread.listener, SIGNAL(seek(qreal)), player, SLOT(seek(qreal)));

        QObject::connect(&inthread.listener, SIGNAL(ping()), &thread.processor, SLOT(ping()));
        QObject::connect(&inthread.listener, SIGNAL(stop()), &thread.processor, SLOT(stop()));

        QObject::connect(player, SIGNAL(durationChanged(qint64)), &thread.processor, SLOT(setDuration(qint64)));
        QObject::connect(player, SIGNAL(positionChanged(qint64)), &thread.processor, SLOT(setPosition(qint64)));

        inthread.start();
    }

    if(player) {
        player->setPriority(decoderIds());
        player->addVideoRenderer(&fr);

        if(!input.isEmpty()) {
            player->play(input);
            player->audio()->setVolume(0);
            player->audio()->setMute(true);
        }
    }

    rthread.start();
    thread.start();

    if(view) {
        view->setGeometry(100, 100, 640, 480);
        view->show();
    }

    int ret = a.exec();

    std::cout << "returned from main event loop" << std::endl;

    thread.quit();
    thread.wait(10000);

    std::cout << "waited for processing thread" << std::endl;

    rthread.quit();
    rthread.wait(1000);

    std::cout << "waited for rendering thread" << std::endl;

    int remaining_now = rthread.controller.framesRemaining();
    int remaining_before = remaining_now+1;

    while(remaining_now > 0 && remaining_now < remaining_before) {
        rthread.wait(1000);
        std::cout << "waited for rendering thread again" << std::endl;
        remaining_before = remaining_now;
        remaining_now = rthread.controller.framesRemaining();
    }

    std::cout << "rendering thread wait over" << std::endl;

    if(view)
        delete view;

    return ret;
}
