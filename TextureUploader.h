/*
    Vidiot id a VIDeo Input Output Transformer With a Touch of Functionality
    Copyright (C) 2016  Delicode Ltd

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
