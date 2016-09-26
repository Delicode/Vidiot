#ifndef FEED_OUTPUT_H
#define FEED_OUTPUT_H

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>
#include <QApplication>
#include <QDebug>

#ifdef SPOUT
#include "external/Spout/Spout.h"
#endif

class FeedOutput : public QObject
{
    Q_OBJECT
public:
    static FeedOutput &instance() {
        static FeedOutput inst;

        return inst;
    }

    QString feedname;

    void ensure(int width, int height);

    void setName(QString name) {
        if(name.startsWith("feed:///"))
            name.remove("feed:///");

        qDebug() << "Output name set to" << name;

        out_name = name;
    }

    QStringList listSources(bool list_self = false);

public slots:
    void send(unsigned int texid, int width = -1, int height = -1, bool flip = true);
    void setResolution(QString resolution_str) {resolution = resolution_str;}

private:
    FeedOutput() : QObject(),
        created(false),
        w(0),
        h(0),
        gl(NULL),
        fbo(NULL)
    {
        gl = new QOpenGLWidget();
        gl->setWindowFlags(Qt::CustomizeWindowHint|Qt::FramelessWindowHint|Qt::Tool);
        gl->setGeometry(0,0,4,4);
        gl->show();
        QApplication::processEvents();
        gl->hide();
        QApplication::processEvents();
    }

    ~FeedOutput() {
        if(gl)
            delete gl;
    }

    /* don't allow copying */
    FeedOutput(FeedOutput const&);
    void operator=(FeedOutput const&);

private:
    bool created;
    int w, h;
#ifdef SPOUT
    SpoutSender spoutserv;
#endif
    QString out_name;

    QOpenGLWidget *gl;
    QOpenGLFramebufferObject *fbo;

    QString resolution;

};

#endif //FEED_OUTPUT_H
