#ifndef VIDEO_VIEW_H
#define VIDEO_VIEW_H

#include "predefines.h"

class FboDrawer {
    bool init_glbuffers;

    QOpenGLShaderProgram *screen_prog;
    QOpenGLBuffer verts;
    QOpenGLBuffer tcoords;
    QOpenGLFunctions_3_2_Core* glfunc;
    GLuint vaoQuad;

public:
    FboDrawer() {
        init_glbuffers = true;
        screen_prog = NULL;
        glfunc = new QOpenGLFunctions_3_2_Core;
        glfunc->initializeOpenGLFunctions();
    }
    ~FboDrawer() {
        if(screen_prog) delete screen_prog;
        if (glfunc) delete glfunc;
    }



    void drawFBO(QOpenGLFramebufferObject *fbo, QSize screen);
    void drawTexture(unsigned int tex_id, int width, int height, bool flip, QSize screen);
    bool ensureBuffers();
};

class VideoView : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_PROPERTY(float pos READ pos WRITE setPos NOTIFY posChanged)
    Q_PROPERTY(float fps READ fps WRITE setFps NOTIFY fpsChanged)
    Q_PROPERTY(float sleeptime READ sleeptime WRITE setSleeptime NOTIFY sleeptimeChanged)
    Q_PROPERTY(float duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(float recordtime READ recordtime WRITE setRecordtime NOTIFY recordtimeChanged)
public:
    VideoView() : QQuickFramebufferObject(),
        _pos(0),
        _fps(30),
        _duration(-1),
        _sleeptime(0),
        _recordtime(0),
        texid(0),
        texwidth(0),
        texheight(0)
    {
        t.setInterval(33);
        t.start();
        connect(&t, SIGNAL(timeout()), this, SLOT(update()));
        setObjectName("vidiot_videoview");
        main_view = NULL;
    }
    Renderer *createRenderer() const;

    void setMainView(QQuickView* v);

    Q_INVOKABLE float pos() {return _pos;}
    Q_INVOKABLE float fps() {return _fps;}
    Q_INVOKABLE float duration() {return _duration;}
    Q_INVOKABLE float sleeptime() {return _sleeptime;}
    Q_INVOKABLE float recordtime() {return _recordtime;}

    unsigned int texid;
    int texwidth;
    int texheight;
    bool texflip;

public Q_SLOTS:
    void setPos(float p) {_pos = p; emit posChanged(_pos);}
    void setFps(float f) {_fps = f; emit fpsChanged(_fps);}
    void setDuration(float p) {_duration = p; emit durationChanged(_duration);}
    void setSleeptime(float p) {_sleeptime = p; emit sleeptimeChanged(_sleeptime);}
    void setRecordtime(float p) {_recordtime = p; emit recordtimeChanged(_recordtime);}
    void receiveError(QString error_str) {
        emit error(error_str);
    }

    void draw(unsigned int tex_id, int width, int height) {
        texid = tex_id;
        texwidth = width;
        texheight = height;
        texflip = true;
    }

    void draw(unsigned int tex_id, int width, int height, bool flip) {
        texid = tex_id;
        texwidth = width;
        texheight = height;
        texflip = flip;
    }

    void receiveSourcesList(QStringList sourcesList);
    void receiveResolutionList(QStringList list);

signals:
    void posChanged(float);
    void fpsChanged(float);
    void durationChanged(float);
    void sleeptimeChanged(float);
    void recordtimeChanged(float);

    void setSource(QString);
    void setFormat(QString);
    void setQuality(QString);
    void setResolution(QString);

    void updateSources();

    void action(QString);

    void record(bool);
    void play(bool);
    void seek(bool, float);

    void error(QString error_str);

    void showCameraProperties();

private:
    QTimer t;

    float _pos;
    float _fps;
    float _duration;
    float _sleeptime;
    float _recordtime;
    QQuickView* main_view;
};

#endif // VIDEO_VIEW_H
