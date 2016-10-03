#ifndef PREDEFINES_H
#define PREDEFINES_H

#define GLER {GLenum er = glGetError(); if(er != GL_NO_ERROR) qDebug() << "GLerror" << __FILE__ << ", " << __LINE__ << er;}

#endif // PREDEFINES_H
