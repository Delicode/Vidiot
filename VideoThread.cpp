#include "VideoThread.h"

#include <QFile>
#include <QOpenGLContext>
#include <QApplication>
#include <QQmlContext>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QDebug>

#include <iostream>

#include "FeedOutput.h"


#ifdef _WIN32
    #include "Spout.h"

    #include <windows.h>
    #include <strmif.h>
    #include <dshow.h>
    #include <dvdmedia.h>

    DEFINE_GUID(CLSID_VideoInputDeviceCategory,0x860bb310,0x5d01,0x11d0,0xbd,0x3b,0x00,0xa0,0xc9,0x11,0xce,0x86);

    void _FreeMediaType(AM_MEDIA_TYPE& mt)
    {
        if (mt.cbFormat != 0)
        {
            CoTaskMemFree((PVOID)mt.pbFormat);
            mt.cbFormat = 0;
            mt.pbFormat = NULL;
        }
        if (mt.pUnk != NULL)
        {
            // pUnk should not be used.
            mt.pUnk->Release();
            mt.pUnk = NULL;
        }
    }


    QStringList CaptureProcessor::listDevices()
    {
        QStringList deviceNames;

        ICreateDevEnum *pDevEnum;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,0,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,(void**)&pDevEnum);
        if(FAILED(hr))
            return deviceNames;
        IEnumMoniker *pEnum;
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnum,0);

        if(FAILED(hr))
        {
            return deviceNames;
        }
        IMoniker *pMoniker = NULL;
        while(S_OK == pEnum->Next(1,&pMoniker,NULL))
        {
            IPropertyBag *pPropBag;
            LPOLESTR str = 0;
            hr = pMoniker->GetDisplayName(0,0,&str);
            if(SUCCEEDED(hr))
            {
                hr = pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void**)&pPropBag);
                if(SUCCEEDED(hr))
                {
                    VARIANT var;
                    VariantInit(&var);

                    hr = pPropBag->Read(L"Description", &var, 0);
                    if (FAILED(hr))
                    {
                        hr = pPropBag->Read(L"FriendlyName", &var, 0);
                    }

                    if(SUCCEEDED(hr)) {
                        QString deviceName = QString::fromWCharArray(var.bstrVal);

                        QStringList deviceModes;

                        IBaseFilter *pFilter;
                        hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
                        if(SUCCEEDED(hr)) {
                            vector<IPin*> pins;
                            IEnumPins *EnumPins;
                            pFilter->EnumPins(&EnumPins);
                            pins.clear();
                            IPin *pin;
                            while(S_OK == EnumPins->Next(1,&pin,NULL))
                            {
                                pins.push_back(pin);
                                pin->Release();
                            }
                            EnumPins->Release();

                            PIN_INFO pInfo;
                            for(int i=0;i<pins.size();i++)
                            {
                                pins[i]->QueryPinInfo(&pInfo);

                                //qDebug() << "Pin" << i  << "dir" << pInfo.dir << QString::fromWCharArray(pInfo.achName);

                                if(pInfo.dir!=1 || QString::fromWCharArray(pInfo.achName) != "Capture" && QString::fromWCharArray(pInfo.achName) != "Output" && QString::fromWCharArray(pInfo.achName) != "Spout Cam")
                                    continue;

                                IEnumMediaTypes *emt=NULL;
                                pins[i]->EnumMediaTypes(&emt);

                                AM_MEDIA_TYPE *pmt;

                                while(S_OK == emt->Next(1,&pmt,NULL))
                                {
                                    if(pmt->formattype == FORMAT_VideoInfo2 && (pmt->cbFormat >= sizeof(VIDEOINFOHEADER2)) && (pmt->pbFormat != NULL) ) {
                                        VIDEOINFOHEADER2 *pVIH = (VIDEOINFOHEADER2*)pmt->pbFormat;

                                        QString compression = "raw";

                                        switch(pVIH->bmiHeader.biCompression) {
                                        case MKTAG('d', 'v', 's', 'd'): compression = "dvvideo"; break;
                                        case MKTAG('M', 'J', 'P', 'G'):
                                        case MKTAG('m', 'j', 'p', 'g'): compression = "mjpeg"; break;
                                        }

                                        REFERENCE_TIME t=pVIH->AvgTimePerFrame; // blocks (100ns) per frame
                                        int FPS=floor(10000000.0/static_cast<double>(t));

                                        QString mode = QString("%2 x %3 @ %4 fps (%1)").arg(compression).arg(pVIH->bmiHeader.biWidth).arg(pVIH->bmiHeader.biHeight).arg(FPS);
                                        if(!deviceModes.contains(mode))
                                            deviceModes.append(mode);

                                    }
                                    else if ( (pmt->formattype == FORMAT_VideoInfo) && (pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) && (pmt->pbFormat != NULL) ) {
                                        VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;

                                        QString compression = "raw";

                                        switch(pVIH->bmiHeader.biCompression) {
                                        case MKTAG('d', 'v', 's', 'd'): compression = "dvvideo"; break;
                                        case MKTAG('M', 'J', 'P', 'G'):
                                        case MKTAG('m', 'j', 'p', 'g'): compression = "mjpeg"; break;
                                        }

                                        REFERENCE_TIME t=pVIH->AvgTimePerFrame; // blocks (100ns) per frame
                                        int FPS=floor(10000000.0/static_cast<double>(t));

                                        QString mode = QString("%2 x %3 @ %4 fps (%1)").arg(compression).arg(pVIH->bmiHeader.biWidth).arg(pVIH->bmiHeader.biHeight).arg(FPS);
                                        if(!deviceModes.contains(mode))
                                            deviceModes.append(mode);
                                    }
                                    else {
                                        OLECHAR* bstrGuid;
                                        StringFromCLSID(pmt->formattype, &bstrGuid);

                                        //qDebug() << deviceName << QString::fromStdWString(bstrGuid);
                                    }

                                    _FreeMediaType(*pmt);
                                }
                                emt->Release();

                                int high_res_mode = -1;
                                int high_res_width = 0;
                                int high_res_height = 0;
                                int high_res_fps = 0;
                                QString high_res_codec = "raw";

                                //find the high resolution options for the device, prefer compressed feed as it's usually faster to process

                                for(int i=0; i<deviceModes.length(); i++) {
                                    QString width = deviceModes.at(i).split(" x ").first();
                                    QString height = deviceModes.at(i).split(" @ ").first().split(" x ").last();
                                    QString fps = deviceModes.at(i).split(" @ ").last().split(" fps ").first();
                                    QString codec = deviceModes.at(i).split("(").last().replace(")", "");

                                    int w = width.toInt();
                                    int h = height.toInt();
                                    int f = fps.toInt();

                                    if(high_res_mode < 0 || high_res_width*high_res_height < w*h || (high_res_width*high_res_height == w*h && (codec != "raw" || f > high_res_fps))) {
                                        high_res_mode = i;
                                        high_res_width = w;
                                        high_res_height = h;
                                        high_res_fps = f;
                                        high_res_codec = codec;
                                    }
                                }

                                if(high_res_mode >= 0)
                                    deviceModes.replace(high_res_mode, "*" + deviceModes.at(high_res_mode));
                            }

                            pins.clear();
                        }

                        if(deviceModes.length() > 1) {
                            for(int i=0; i<deviceModes.length(); i++) {
                                if(i == 0)
                                    deviceNames.append("dshow:///" + deviceName + "?!" + deviceModes.at(i));
                                else
                                    deviceNames.append("dshow:///" + deviceName + "?" + deviceModes.at(i));
                            }
                        }
                        else
                            deviceNames.append("dshow:///" + deviceName);
                    }
                    VariantClear(&var);
                    pPropBag->Release();
                } else {
                    qDebug() << "could not bind to storage";
                }
            }
            pMoniker->Release();
        }
        pEnum->Release();
        pDevEnum->Release();

        return deviceNames;
    }
#else
    QStringList CaptureProcessor::listDevices()
    {
        return QStringList();
    }

#endif //_WIN32

CaptureProcessor::CaptureProcessor() : QObject(NULL),
    player(NULL),
    recorder(NULL),
    feed_input(NULL),
    video_view(NULL),
    main_view(NULL),
    resolution("100%"),
    recording(false),
    ping_required(false)
{
    setObjectName("Capture processor");

    gl = new QOpenGLWidget();

    gl->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::Tool);
    gl->setGeometry(0,0,4,4);
    gl->show();
    QApplication::processEvents();
    gl->hide();
    QApplication::processEvents();

    if(!gl->context())
        qDebug() << "No gl context";

    uploader = new TextureUploader(gl);
    downloader = new TextureDownloader(gl);
}

void CaptureProcessor::ping()
{
    ping_time.restart();
}

CaptureProcessor::~CaptureProcessor()
{
    if(player) delete player;
    if(recorder) delete recorder;
    if(feed_input) delete feed_input;
    if(downloader) delete downloader;
    if(uploader) delete uploader;
}

void CaptureProcessor::updateSourceList()
{
    if(!main_view)
        return;

    QStringList sourcelist;

    sourcelist.append("None");
    sourcelist.append("Open file...");

    QStringList devs = listDevices();

    if(devs.length())
        sourcelist.append(devs);

    QStringList feeds = FeedOutput::instance().listSources();
    if(feeds.length())
        sourcelist.append(feeds);

    QSettings settings;
    QStringList recent_files = settings.value("recent_files").toStringList();

    if(recent_files.length()) {
        sourcelist.append("#Recent files ?clear");
        sourcelist.append(recent_files);
    }

    if (sourcelist != current_sourcelist)
        emit sourcesListUpdate(sourcelist);

    current_sourcelist = sourcelist;
}

void CaptureProcessor::action(QString action_str)
{
    if(action_str == "#Recent files ?clear") {
        QSettings settings;
        settings.remove("recent_files");

        updateSourceList();
    }
}

void CaptureProcessor::updateResolutionList(int w, int h)
{
    if(!main_view)
        return;

    QStringList list;

    if(w > 0 && h > 0) {
        list.append(QString("100% - %1 x %2").arg(w).arg(h));
        list.append(QString("50% - %1 x %2").arg(w/2).arg(h/2));
        list.append(QString("25% - %1 x %2").arg(w/4).arg(h/4));
        list.append("1080p");
        list.append("720p");
        list.append("Custom");
    }
    else {
        list.append("100%");
        list.append("50%");
        list.append("25%");
        list.append("1080p");
        list.append("720p");
        list.append("Custom");
    }

    if(main_view && list != current_resolutionlist)
        main_view->rootContext()->setContextProperty("resolutionlist", list);

    current_resolutionlist = list;
}

void CaptureProcessor::record(bool enable)
{
    recording = enable;

    if(!recording)
        emit stopRecording();
}

void CaptureProcessor::record(QString filename)
{
    recording = true;
    record_filename = filename;
}

void CaptureProcessor::play(bool enable)
{
    if(player) {
        player->playing = enable;

        if(enable) {
            player->t.start();
            qDebug() << "Playing started";
        }
        else {
            player->starttime = player->starttime + player->t.nsecsElapsed();
            qDebug() << "Playing stopped at" << player->starttime/1000000000.f << "s";
        }
    }
}

void CaptureProcessor::rewind()
{
    if(player)
        player->rewind = true;
}

void CaptureProcessor::seek(bool enable, float pos)
{
    if(player) {
        if(pos < 0) pos = 0;
        if(pos > 1) pos = 1;
        player->next_seek_time_s = pos*player->duration;
    }

    if(player->seeking != enable) {
        player->seeking = enable;
        if(!enable) {
            player->starttime = player->frame_time_s*1000000000.f;
            player->t.start();
            player->next_seek_time_s = -1;
        }
    }
}

void CaptureProcessor::setSource(QString sourcename)
{
    qDebug() << "Source selected" << sourcename;

    if(sourcename == current_sourcename)
        return;

    prev_sourcename = current_sourcename;
    current_sourcename = sourcename;

    if(sourcename.startsWith("file:///")) {
        QSettings settings;
        QStringList recent_files = settings.value("recent_files").toStringList();
        while (recent_files.length() >= 10)
            recent_files.removeLast();

        if(recent_files.contains(sourcename))
            recent_files.removeAll(sourcename);

        recent_files.prepend(sourcename);

        settings.setValue("recent_files", recent_files);
    }


    updateSourceList();
    updateResolutionList(0, 0);
}

void CaptureProcessor::setResolution(QString resolution_str)
{
    resolution = resolution_str;
    qDebug() << "Resolution set to" << resolution;
}


void CaptureProcessor::stop()
{
    recording = false;
}

void CaptureProcessor::process()
{
    t.start();

    if(ping_required && !ping_time.isValid())
        ping_time.start();

    qint64 capture_start = 0;
    qint64 download_start = 0;
    qint64 record_start = 0;
    qint64 sleep_start = 0;
    qint64 sleep_end = 0;


    if(ping_time.isValid() && ping_time.elapsed() > 5000) {
        emit quit();
        qApp->quit();
        return;
    }

    if(current_sourcename != prev_sourcename) {
        prev_sourcename = current_sourcename;

        if(feed_input) {
            delete feed_input;
            feed_input = NULL;
            emit sendResolution(0,0);
        }

        if(player) {
            delete player;
            player = NULL;
            emit sendResolution(0,0);
        }

        if(current_sourcename == "None" || current_sourcename.startsWith("file:///")) {

        }
        else if(current_sourcename.startsWith("dshow:///")) {
            player = new VideoPlayer(current_sourcename);
            if(player->error) {
                emit error(player->error_str);
                std::cout << "Error:" << player->error_str.toLocal8Bit().data() << std::endl;
                delete player;
                player = NULL;
            }
            else {
                player->moveToThread(this->thread());
                connect(this, SIGNAL(showCameraProperties()), player, SLOT(showProperties()));
            }
        }
        else if(current_sourcename.startsWith("feed:///")) {
            feed_input = new FeedInput(current_sourcename.split("feed:///").last(), gl);
            feed_input->moveToThread(this->thread());
        }
    }

    if(player) {
        player->play();

        uploader->upload(resolution, player->width, player->height, player->data);

        if(uploader->preroll == 0) {
            emit sendFeed(uploader->fbo()->texture(), uploader->fbo()->width(), uploader->fbo()->height(), false);
            emit sendFeedFbo(uploader->fbo()->handle(), uploader->fbo()->width(), uploader->fbo()->height(), false);
            emit sendResolution(player->width, player->height);
        }

        if(recording && player->playing) {
            if(!record_time.isValid())
                record_time.start();

            RecorderFrame frame(player->width, player->height, new unsigned char[player->width*player->height*3], record_time.elapsed());
            memcpy(frame.data, player->data, frame.width*frame.height*3*sizeof(unsigned char));
            emit newRecorderFrame(frame);
        }
    }
    else if(feed_input) {
        capture_start = t.nsecsElapsed();
        feed_input->capture(resolution);
        if (feed_input->fbo())
        {
            emit sendFeed(feed_input->fbo()->texture(), feed_input->fbo()->width(), feed_input->fbo()->height(), true);
            emit sendFeedFbo(feed_input->fbo()->handle(), feed_input->fbo()->width(), feed_input->fbo()->height(), true);
            emit sendResolution(feed_input->input_width, feed_input->input_height);

            if(recording) {
                download_start = t.nsecsElapsed();
                downloader->download(feed_input->fbo());

                record_start = t.nsecsElapsed();
                //skip first frame as the download buffer is not yet cycled
                if(downloader->preroll == 0) {
                    if(!record_time.isValid())
                        record_time.start();

                    RecorderFrame frame(downloader->buf_width, downloader->buf_height, new unsigned char[downloader->buf_width*downloader->buf_height*3], record_time.elapsed());
                    memcpy(frame.data, downloader->data, frame.width*frame.height*3*sizeof(unsigned char));
                    emit newRecorderFrame(frame);
                }
            }
        }

        sleep_start = t.nsecsElapsed();

        float sleeptime_ms = feed_input->getSleeptime_ms();

        emit updateSleeptime(feed_input->avg_sleeptime_ms);

        if(sleeptime_ms > 0)
            QThread::usleep(1000*sleeptime_ms);

        sleep_end = t.nsecsElapsed();
    }

    if(recording) {
        emit updateRecordtime((float)record_time.elapsed()/1000.f);
    }
    else {
        record_time.invalidate();
        emit updateRecordtime(0);
    }

    if(recording) {
        qDebug() << "";
        qDebug() << "Total:      " << (float)t.nsecsElapsed()/1000000.f;
        qDebug() << "   Capture :" << (float)(download_start - capture_start)/1000000.f;
        qDebug() << "   Download:" << (float)(record_start - download_start)/1000000.f;
        qDebug() << "   Write   :" << (float)(sleep_start - record_start)/1000000.f;
        qDebug() << "   Sleep   :" << (float)(sleep_end - sleep_start)/1000000.f;
    }

    if(player || feed_input) {
        float dt = (float)t.nsecsElapsed()/1000000.f;
        t.restart();
        if(dt > 0) {
            float new_fps = 1000.f/(float)dt;

            fps.append(new_fps);

            while(fps.length() > 30)
                fps.removeFirst();

            QList<float> sorted_fps = fps;
            qSort(sorted_fps);

            emit updateFPS(sorted_fps[sorted_fps.length()/2]);
        }
    }
}

void CaptureProcessor::setDuration(qint64 duration_ms)
{
    std::cout << "playing:" << ((float)duration_ms)*0.001f << std::endl;
}

void CaptureProcessor::setPosition(qint64 position_ms)
{
    std::cout << "playpos:" << ((float)position_ms)*0.001f << std::endl;
}

void CaptureProcessor::setVideoView(VideoView *vv)
{
    if(!vv)
        return;

    video_view = vv;


}

void CaptureProcessor::setMainView(QQuickView *mv)
{
    if(!mv)
        return;

    main_view = mv;
    updateSourceList();
    updateResolutionList();
    if (main_view)
        main_view->rootContext()->setContextProperty("sourcelist", current_sourcelist);
}



CaptureThread::CaptureThread() : QThread()
{
    setObjectName("Capture thread");

    processor.moveToThread(this);
    processor.gl->context()->moveToThread(this);
    runtimer.moveToThread(this);

    processor.uploader->moveToThread(this);
    processor.downloader->moveToThread(this);

    connect(&processor, SIGNAL(quit()), this, SLOT(quit()));
    connect(&runtimer, SIGNAL(timeout()), &processor, SLOT(process()));
    setObjectName("capturethread");
}

void CaptureThread::run()
{
    runtimer.start(1);

    int ret = exec();

    runtimer.stop();
}

RecordThread::RecordThread() : QThread()
{
    setObjectName("Record thread");

    controller.moveToThread(this);
    runtimer.moveToThread(this);

    connect(&runtimer, SIGNAL(timeout()), &controller, SLOT(process()));
    setObjectName("recordthread");
}

void RecordThread::run()
{
    runtimer.start(1);

    int ret = exec();

    runtimer.stop();
    controller.stop();
}
