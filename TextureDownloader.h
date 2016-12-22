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

#ifndef TEXTURE_DOWNLOADER_H
#define TEXTURE_DOWNLOADER_H

#include "predefines.h"

#define RECORD_BUFS 20

class TextureDownloader : public QObject
{
	Q_OBJECT
public:
	TextureDownloader(QOpenGLWidget *glwid);
	virtual ~TextureDownloader();

	void download(QOpenGLFramebufferObject *fbo);

	void ensureData(int width, int height);

	QOpenGLWidget *gl;

    QOpenGLBuffer *bufs[RECORD_BUFS];
	int download_index;

	unsigned char *data;
	unsigned long long datasize;

	unsigned int buf_width, buf_height;

	int preroll;
};

#endif //TEXTURE_DOWNLOADER_H
