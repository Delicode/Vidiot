#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>
}

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    VideoPlayer(QString filename);
    virtual ~VideoPlayer();

    bool running;
    bool error;


    int width, height;
    float duration;
    float frame_time_s;

    unsigned char *data;

    bool playing;
    qint64 starttime;

    bool rewind;

    bool seeking;
    float seek_time_s;
    float next_seek_time_s;

    float avg_sleeptime_ms;

    QString playfile;

    QElapsedTimer t;

    QString error_str;

    QString device_name;

    float getSleeptime_ms();

public slots:
    void play();
    void showProperties();

private:
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodecContext *pCodecCtxOrig;
    int videoStream;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    struct SwsContext *sws_ctx;

    bool readFrame();
};

#endif //VIDEOPLAYER_H
