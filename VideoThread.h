#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>
#include <QOpenGLWidget>
#include <QMutex>
#include <QQuickView>
#include <QElapsedTimer>

#include "VideoPlayer.h"
#include "TextureUploader.h"
#include "TextureDownloader.h"
#include "FeedInput.h"
#include "VideoRecorder.h"
#include "VideoView.h"



class CaptureProcessor : public QObject
{
    Q_OBJECT
public:
    CaptureProcessor();
    ~CaptureProcessor();

    void setVideoView(VideoView *vv);
    void setMainView(QQuickView *mv);
    void setPingRequired(bool required) {ping_required = required;}

    static QStringList listDevices();

    QOpenGLWidget *gl;

    VideoPlayer *player;
    TextureUploader *uploader;
    TextureDownloader *downloader;
    FeedInput *feed_input;
    VideoRecorder *recorder;
    VideoView *video_view;
    QQuickView *main_view;
    bool recording;

    QString current_sourcename;
    QString prev_sourcename;
    QStringList current_sourcelist;
    QStringList current_qualitylist;
    QStringList current_resolutionlist;

    QString record_filename;

    QElapsedTimer ping_time;

    QElapsedTimer record_time;

    QElapsedTimer t;
    QList<float> fps;

public slots:
    void process();

    void updateSourceList();
    void setSource(QString sourcename);
    void stop();
    void setResolution(QString resolution_str);
    void record(bool enable);
    void record(QString filename);
    void play(bool enable);
    void seek(bool enable, float pos);
    void rewind();

    void action(QString action_str);

    void updateResolutionList(int w = 0, int h = 0);

    void ping();

    void setDuration(qint64 duration_ms);
    void setPosition(qint64 position_ms);

    void cameraPropertiesRequested() {emit showCameraProperties();}

signals:
    void sourceError(QString reason);
    void pos(float time_s);
    void updateFPS(float fps);
    void updateSleeptime(float time_ms);
    void updateRecordtime(float time_s);
    void playVideo(QString filename);

    void sendFeed(unsigned int, int, int, bool);
    void sendFeedFbo(unsigned int, int, int, bool);
    void sendResolution(int,int);
    void error(QString error_str);

    void showCameraProperties();

    void quit();
    void stopRecording();
    void newRecorderFrame(RecorderFrame fr);

private:
    QString resolution;

    bool ping_required;
};

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    CaptureThread();
    void run();
    CaptureProcessor processor;
    QTimer runtimer;
};

class RecordThread : public QThread
{
    Q_OBJECT
public:
    RecordThread();
    void run();
    RecorderController controller;
    QTimer runtimer;
};

#endif //VIDEOTHREAD_H
