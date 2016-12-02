#include "TextureUploader.h"

TextureUploader::TextureUploader(QOpenGLWidget *glwid) : QObject(NULL),
    subdata(NULL),
    totsubdata(0),
    output_index(0),
    input_fbo(NULL),
    gl(glwid),
    preroll(2)
{
    for(int i=0; i<2; i++)
        bufs[i] = NULL;

    for(int i=0; i<3; i++)
        output_fbo[i] = NULL;
}

TextureUploader::~TextureUploader()
{
    for(int i=0; i<3; i++) {
        if(output_fbo[i])
            delete output_fbo[i];
    }

    if(subdata)
        delete [] subdata;
}

void TextureUploader::ensureInputFBO(int width, int height)
{
    if(input_fbo == NULL || input_fbo->width() != width || input_fbo->height() != height) {

        preroll = 2;

//        std::cout << "GL Vendor:   " << (const char *)glGetString(GL_VENDOR) << std::endl;
//        std::cout << "GL Renderer: " << (const char *)glGetString(GL_RENDERER) << std::endl;
//        std::cout << "GL Version: " << (const char *)glGetString(GL_VERSION) << std::endl;

        if(input_fbo)
            delete input_fbo;

        input_fbo = new QOpenGLFramebufferObject(width, height, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, input_fbo->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        //qDebug() << "FBO" << width << "x" << height;

        unsigned char *temp = new unsigned char[3*width*height];
        memset(temp, 0, 3*width*height*sizeof(unsigned char));
        for(int i=0; i<2; i++) {
            if(bufs[i])
                delete bufs[i];
            bufs[i] = new QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
            bufs[i]->setUsagePattern(QOpenGLBuffer::StreamDraw);
            bufs[i]->create();
            bufs[i]->bind();
            bufs[i]->allocate(temp, 3*width*height*sizeof(unsigned char));
            bufs[i]->release();
        }
        delete [] temp;
        upload_index = 0;
    }
}

void TextureUploader::ensureOutputFBO(int width, int height)
{
    if(output_fbo[output_index] == NULL || output_fbo[output_index]->width() != width || output_fbo[output_index]->height() != height) {
        if(output_fbo[output_index])
            delete output_fbo[output_index];

        output_fbo[output_index] = new QOpenGLFramebufferObject(width, height, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, output_fbo[output_index]->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        //qDebug() << "Data FBO" << width << "x" << height;
    }
}

void TextureUploader::ensureSubdata(int width, int height)
{
    if(subdata == NULL || totsubdata != width*height*3) {
        if(subdata)
            delete [] subdata;

        subdata = new unsigned char[width*height*3];
        totsubdata = width*height*3;
        memset(subdata, 0, width*height*3*sizeof(unsigned char));
    }
}

void TextureUploader::upload(QString resolution, int width, int height, unsigned char *data)
{
    int w = width;
    int h = height;
    int subsample = 0;
    bool blit = false;

    if(resolution.startsWith("50%")) {
        w = w/2;
        h = h/2;
        subsample = 2;
    }
    else if(resolution.startsWith("25%")) {
        w = w/4;
        h = h/4;
        subsample = 4;
    }
    else if(resolution.startsWith("1080p")) {
        w = 1920;
        h = 1080;
        if(w != width || h != height)
            blit = true;
    }
    else if(resolution.startsWith("720p")) {
        w = 1280;
        h = 720;
        if(w != width || h != height)
            blit = true;
    }
    else if(resolution.startsWith("Custom")) {
        // Expect the format "Custom - w x h"
        QString res = resolution;
        res.remove("Custom - ");
        res.remove(' ');
        QStringList r = res.split("x");
        if(r.length() == 2) {
            bool wok, hok;
            int tw = r.at(0).toInt(&wok);
            int th = r.at(1).toInt(&hok);
            if(wok && hok) {
                w = tw;
                h = th;
                if(w != width || h != height)
                    blit = true;
            }
        }
    }

    output_index = (output_index + 1) % 3;

    gl->makeCurrent();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    if(subsample){
        ensureInputFBO(w, h);
        ensureSubdata(w, h);

        int ss3 = subsample*3;
        unsigned char *d = data;
        unsigned char *s = subdata;
        for(int j=0; j<h; j++) {
            d = data + j*ss3*width;
            for(int i=0; i<w; i++, s+=3, d+=ss3) {
                s[0] = d[0];
                s[1] = d[1];
                s[2] = d[2];
            }
        }

        if(bufs[(upload_index+1)%2]->bind()) {
            f->glBindTexture(GL_TEXTURE_2D, input_fbo->texture());
            f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            f->glBindTexture(GL_TEXTURE_2D, 0);
        }

        if(bufs[upload_index]->bind()) {
            f->glBufferData(GL_PIXEL_UNPACK_BUFFER, w*h*3*sizeof(unsigned char), NULL, GL_STREAM_DRAW);
            unsigned char *new_data = (unsigned char*)bufs[upload_index]->map(QOpenGLBuffer::WriteOnly);
            if(new_data)
                memcpy(new_data, subdata, w*h*3*sizeof(unsigned char));
            bufs[upload_index]->unmap();

//            f->glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, w*h*3*sizeof(unsigned char), data);
        }



        bufs[upload_index]->release();
        upload_index = (upload_index + 1) % 2;
    }
    else {
        ensureInputFBO(width, height);

        if(bufs[(upload_index+1)%2]->bind()) {
            f->glBindTexture(GL_TEXTURE_2D, input_fbo->texture());
            f->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, input_fbo->width(), input_fbo->height(), GL_RGB, GL_UNSIGNED_BYTE, NULL);
            f->glBindTexture(GL_TEXTURE_2D, 0);
        }

        if(bufs[upload_index]->bind()) {
            f->glBufferData(GL_PIXEL_UNPACK_BUFFER, input_fbo->width()*input_fbo->height()*3*sizeof(unsigned char), NULL, GL_STREAM_DRAW);
            unsigned char *new_data = (unsigned char*)bufs[upload_index]->map(QOpenGLBuffer::WriteOnly);
            if(new_data)
                memcpy(new_data, data, input_fbo->width()*input_fbo->height()*3*sizeof(unsigned char));
            bufs[upload_index]->unmap();

            //f->glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, input_fbo->width()*input_fbo->height()*3*sizeof(unsigned char), data);
        }
        bufs[upload_index]->release();

        upload_index = (upload_index + 1) % 2;
    }

    ensureOutputFBO(w, h);

    float output_aspect = (float)output_fbo[output_index]->width()/(float)output_fbo[output_index]->height();
    float input_aspect = (float)input_fbo->width()/(float)input_fbo->height();
    int x = 0;
    int y = 0;
    int sw = output_fbo[output_index]->width();
    int sh = output_fbo[output_index]->height();
    if(input_aspect < output_aspect) {
        sw = input_fbo->width()*output_fbo[output_index]->height()/input_fbo->height();
        x = (output_fbo[output_index]->width()-sw)/2;
    }
    else {
        sh = input_fbo->height()*output_fbo[output_index]->width()/input_fbo->width();
        y = (output_fbo[output_index]->height()-sh)/2;
    }

    input_fbo->bind();
    input_fbo->blitFramebuffer(output_fbo[output_index], QRect(x, y, sw, sh), input_fbo, QRect(0, 0, input_fbo->width(), input_fbo->height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
    input_fbo->release();

    if(preroll)
        preroll--;

    glFinish();

    gl->doneCurrent();
}
