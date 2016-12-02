#ifndef PREDEFINES_H
#define PREDEFINES_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QPainter>
#include <QTimer>
#include <QElapsedTimer>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QSettings>
#include <QtConcurrentRun>
#include <QBuffer>
#include <QScreen>

#include <QQmlContext>
#include <QQuickView>
#include <QQuickFramebufferObject>

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include <iostream>
#include <math.h>

#include <QtAV>

#define GLER {GLenum er = glGetError(); if(er != GL_NO_ERROR) qDebug() << "GLerror" << __FILE__ << ", " << __LINE__ << er;}

#endif // PREDEFINES_H
