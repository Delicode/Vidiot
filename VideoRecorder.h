#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include "predefines.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
    #include <libavdevice/avdevice.h>
    #include <libavformat/avio.h>
}

typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;
    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;
    AVFrame *frame;
    AVFrame *tmp_frame;
    float t, tincr, tincr2;
    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

struct RecorderFrame {
    unsigned char *data;
    int width, height;
    int time_ms;

    RecorderFrame(int w = 0, int h = 0, unsigned char *d = NULL, int t = 0)
    {
        width = w;
        height = h;
        data = d;
        time_ms = t;
    }
};

class VideoRecorder
{
public:
    VideoRecorder(QString output_filename, QString quality_name, QString format);
    virtual ~VideoRecorder();

    bool error;
    bool done;

    QString quality;

    float frame_time_s;


    int w, h;

    void initialize(int width, int height);

    void flush();
    void flushOne();
    void end();

    static QStringList list_codecs(bool log = false);
    bool writeFrame(const RecorderFrame &frame);

    static AVCodecID codec_h264_qsv;
    static AVCodecID codec_prores;
    static AVCodecID codec_libvpxvp9;
    static AVCodecID codec_libx264;
    static AVCodecID codec_nvenc_h264;
    static AVCodecID codec_libtheora;

private:
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    OutputStream video_st;
    AVCodec *video_codec;
    AVDictionary *opt;

    AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height);


    QElapsedTimer t;
    int framecount;
    QString filename;
};


class RecorderController : public QObject
{
    Q_OBJECT
public:
    RecorderController();

public slots:
    void process();
    void stop();
    void recorderFrame(RecorderFrame fr);
    void setFilename(QString fn) {filename = fn;}
    void setFormat(QString format_str);
    void setQuality(QString quality_str);
    int framesRemaining() {return frames.length();}

private:
    QList<RecorderFrame> frames;
    VideoRecorder *recorder;
    QString filename;
    QString quality;
    QString format;
};


#endif //VIDEORECORDER_H
