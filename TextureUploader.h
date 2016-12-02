#ifndef TEXTURE_UPLOADER_H
#define TEXTURE_UPLOADER_H

#include "predefines.h"

class TextureUploader : public QObject
{
    Q_OBJECT
public:
    TextureUploader(QOpenGLWidget *glwid);
    virtual ~TextureUploader();

    QOpenGLWidget *gl;
    QOpenGLFramebufferObject *output_fbo[3];

    void ensureInputFBO(int width, int height);
    void ensureOutputFBO(int width, int height);
    void ensureSubdata(int width, int height);

    void upload(QString resolution, int width, int height, unsigned char *data);

    QOpenGLFramebufferObject *fbo() {return output_fbo[output_index];}

    int preroll;

private:

    QOpenGLFramebufferObject *input_fbo;
    int output_index;
    unsigned char *subdata;
    int totsubdata;

    QOpenGLBuffer *bufs[2];
    int upload_index;
};


#endif //TEXTURE_UPLOADER_H
