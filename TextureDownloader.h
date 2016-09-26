#ifndef TEXTURE_DOWNLOADER_H
#define TEXTURE_DOWNLOADER_H

#include <QOpenGLWidget>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>

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
