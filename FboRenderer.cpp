#include "FboRenderer.h"
#include "QtAV/private/VideoRenderer_p.h"
#include "QtAV/private/mkid.h"

#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QOpenGLFramebufferObject>
#include <QThread>

#include "FeedOutput.h"

#ifdef QT_OPENGL_DYNAMIC
#include <QtGui/QOpenGLFunctions>
#define DYGL(glFunc) QOpenGLContext::currentContext()->functions()->glFunc
#else
#define DYGL(glFunc) glFunc
#endif

namespace QtAV {
static const VideoRendererId VideoRendererId_FboRenderer = mkid::id32base36_4<'F','B','O','R'>::value;

class FboRendererPrivate : public VideoRendererPrivate
{
public:
    FboRendererPrivate() :
        VideoRendererPrivate()
      , frame_changed(false)
      , source(0)
      , glctx(0)
      , fbo_index(0)
    {
        for(int i=0; i<3; i++)
            fbo[i] = NULL;
    }

    bool frame_changed;
    QObject *source;
    QOpenGLContext *glctx;
    OpenGLVideo glv;

    QOpenGLFramebufferObject *fbo[3];

    int fbo_index;
};

FboRenderer::FboRenderer() :
    VideoRenderer(*new FboRendererPrivate()),
    paused(false),
    seek_time(-1)
{
    setPreferredPixelFormat(VideoFormat::Format_YUV420P);

    gl = new QOpenGLWidget();
    gl->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::Tool);
    gl->setGeometry(0,0,4,4);
    gl->show();
    QApplication::processEvents();
    gl->hide();
    QApplication::processEvents();

    resolution = "100%";
}

VideoRendererId FboRenderer::id() const
{
    return VideoRendererId_FboRenderer;
}

bool FboRenderer::isSupported(VideoFormat::PixelFormat pixfmt) const
{
    if (pixfmt == VideoFormat::Format_RGB48BE)
        return false;

    return OpenGLVideo::isSupported(pixfmt);
}

bool FboRenderer::receiveFrame(const VideoFrame &frame)
{
    DPTR_D(FboRenderer);
    d.video_frame = frame;
    d.frame_changed = true;

    QCoreApplication::postEvent(this, new QEvent(QEvent::User));
    return true;
}

void FboRenderer::drawFrame()
{
    if(!t.isValid())
        t.start();

    DPTR_D(FboRenderer);

    if (d.glctx != QOpenGLContext::currentContext()) {
        d.glctx = QOpenGLContext::currentContext();
        d.glv.setOpenGLContext(d.glctx);
    }

    int original_w = d.video_frame.isValid() ? d.video_frame.width() : 640;
    int original_h = d.video_frame.isValid() ? d.video_frame.height() : 480;

    int w = original_w;
    int h = original_h;

    if(resolution.startsWith("50%")) {
        w = w/2;
        h = h/2;
    }
    else if(resolution.startsWith("25%")) {
        w = w/4;
        h = h/4;
    }
    else if(resolution.startsWith("1080p")) {
        w = 1920;
        h = 1080;
    }
    else if(resolution.startsWith("720p")) {
        w = 1280;
        h = 720;
    }



    if(d.fbo[d.fbo_index] == NULL || w != d.fbo[d.fbo_index]->width() || h != d.fbo[d.fbo_index]->height()) {
        if(d.fbo[d.fbo_index] != NULL)
            delete d.fbo[d.fbo_index];

        d.fbo[d.fbo_index] = new QOpenGLFramebufferObject(w, h);
    }

    if(d.fbo[d.fbo_index]->bind()) {
        DYGL(glViewport(0, 0, d.fbo[d.fbo_index]->width(), d.fbo[d.fbo_index]->height()));
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        float output_aspect = (float)w/(float)h;
        float input_aspect = (float)original_w/(float)original_h;
        int x = 0;
        int y = 0;
        int sw = w;
        int sh = h;
        if(input_aspect < output_aspect) {
            sw = original_w*h/original_h;
            x = (w-sw)/2;
        }
        else {
            sh = original_h*w/original_w;
            y = (h-sh)/2;
        }

        d.glv.setProjectionMatrixToRect(QRect(x, y, w-2*x, h-2*y));

        if (!d.video_frame.isValid()) {
            d.glv.fill(QColor(0, 0, 0, 0));
            return;
        }
        if (d.frame_changed) {
            d.glv.setCurrentFrame(d.video_frame);
            d.frame_changed = false;
        }
        d.glv.render();

        d.fbo[d.fbo_index]->release();

        emit sendFeed(d.fbo[d.fbo_index]->texture(), d.fbo[d.fbo_index]->width(), d.fbo[d.fbo_index]->height());
        emit sendResolution(original_w, original_h);

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

    d.fbo_index = (d.fbo_index+1)%3;
}

bool FboRenderer::event(QEvent *e)
{
    if (e->type() != QEvent::User)
        return FboRenderer::event(e);
    gl->makeCurrent();
    drawFrame();
    gl->doneCurrent();

    if(paused)
        emit pause(true);

    return true;
}

void FboRenderer::setPause(bool p)
{
    paused = p;
}

void FboRenderer::setSeek(qreal s)
{
    seek_time = s;
}

}
