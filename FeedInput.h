#ifndef FEED_INPUT_H
#define FEED_INPUT_H

#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>
#include <QMutex>
#include <QOpenGLBuffer>
#include <QElapsedTimer>
#include "External/Spout/Spout.h"

class FeedInput : public QObject
{
    Q_OBJECT
public:
    FeedInput(QString name, QOpenGLWidget *glwid);
    virtual ~FeedInput();

    QOpenGLWidget *gl;
    QOpenGLFramebufferObject *input_fbo;
    QOpenGLFramebufferObject *output_fbo[3];
    int output_index;

    QOpenGLFramebufferObject *fbo() {return output_fbo[output_index];}

    void start();

    void ensureSpout();
    void ensureFBO();
    void capture(QString resolution);



    QString spout_name;

    unsigned int input_width, input_height;
    unsigned int output_width, output_height;


    float avg_sleeptime_ms;

    bool first_frame;
    bool error;


    float fps;
    long long framecount;

    float getSleeptime_ms();

private:
    bool spoutreceiver_created;
    SpoutReceiver spoutreceiver;

    QElapsedTimer t;
};


#endif //FEED_INPUT_H
