#ifndef FBO_RENDERER_H
#define FBO_RENDERER_H

#include <QtAV>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>

namespace QtAV {

class FboRendererPrivate;

class FboRenderer : public QObject, public VideoRenderer
{
    Q_OBJECT
    Q_DISABLE_COPY(FboRenderer)
    DPTR_DECLARE_PRIVATE(FboRenderer)
public:
    explicit FboRenderer();
    ~FboRenderer() {
        if (gl_func)
            delete gl_func;
    }

    VideoRendererId id() const Q_DECL_OVERRIDE;
    bool isSupported(VideoFormat::PixelFormat pixfmt) const Q_DECL_OVERRIDE;

    QOpenGLWidget *gl;
    QOpenGLFunctions_3_2_Core* gl_func;

public slots:
    void setResolution(QString resolution_str) {resolution = resolution_str;}
    void setPause(bool);
    void setSeek(qreal);

signals:
    void sendFeed(unsigned int texid, int width = -1, int height = -1);
    void sendResolution(int, int);
    void updateFPS(float);
    void pause(bool);
    void seek(qreal);

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    bool receiveFrame(const VideoFrame &frame) Q_DECL_OVERRIDE;
    void drawFrame() Q_DECL_OVERRIDE;

    QString resolution;
    QElapsedTimer t;
    QList<float> fps;

    bool paused;
    qreal seek_time;
};
}

#endif //SPOUT_RENDERER_H
