#include "TextureDownloader.h"

#include <QOpenGLFunctions>

TextureDownloader::TextureDownloader(QOpenGLWidget *glwid) : QObject(NULL),
    gl(glwid),
    preroll(2),
    data(NULL),
    datasize(0),
    download_index(0),
    buf_width(0),
    buf_height(0)
{
    for(int i=0; i<RECORD_BUFS; i++)
        bufs[i] = NULL;
}

TextureDownloader::~TextureDownloader()
{
    for(int i=0; i<RECORD_BUFS; i++) {
        if(bufs[i])
            delete bufs[i];
    }

    if(data)
        delete [] data;
}

void TextureDownloader::download(QOpenGLFramebufferObject *fbo)
{
    gl->makeCurrent();
    ensureData(fbo->width(), fbo->height());

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    fbo->bind();
    bufs[download_index]->bind();
    bufs[download_index]->allocate(3*fbo->width()*fbo->height()*sizeof(unsigned char));
    f->glReadPixels(0, 0, fbo->width(), fbo->height(), GL_RGB, GL_UNSIGNED_BYTE, NULL);
    bufs[download_index]->release();
    fbo->release();

    bufs[(download_index+1)%RECORD_BUFS]->bind();
    //bufs[(download_index+1)%RECORD_BUFS]->read(0, data, 3*fbo->width()*fbo->height()*sizeof(unsigned char));
    unsigned char *new_data = (unsigned char*)bufs[(download_index+1)%2]->map(QOpenGLBuffer::ReadOnly);
    memcpy(data, new_data, fbo->width()*fbo->height()*3*sizeof(unsigned char));
    bufs[(download_index+1)%RECORD_BUFS]->unmap();
    bufs[(download_index+1)%RECORD_BUFS]->release();

    download_index = (download_index + 1) % RECORD_BUFS;

    if(preroll)
        preroll--;

    gl->doneCurrent();
}

void TextureDownloader::ensureData(int width, int height)
{
    if(data == NULL || datasize != width*height*3) {
        if(data)
            delete [] data;

        if(width > 0 && height > 0)
            data = new unsigned char[width*height*3];
        else
            data = NULL;
    }

    if(buf_width != width || buf_height != height) {

        buf_width = width;
        buf_height = height;

        unsigned char *temp = new unsigned char[3*width*height];
        memset(temp, 0, 3*width*height*sizeof(unsigned char));
        for(int i=0; i<RECORD_BUFS; i++) {
            if(bufs[i])
                delete bufs[i];
            bufs[i] = new QOpenGLBuffer(QOpenGLBuffer::PixelPackBuffer);
            bufs[i]->setUsagePattern(QOpenGLBuffer::StreamRead);
            bufs[i]->create();
            bufs[i]->bind();
            bufs[i]->allocate(temp, 3*width*height*sizeof(unsigned char));
            bufs[i]->release();
        }
        delete [] temp;
        download_index = 0;

        preroll = RECORD_BUFS;
    }
}
