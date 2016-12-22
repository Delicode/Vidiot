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

#ifndef FBO_RENDERER_H
#define FBO_RENDERER_H

#include "predefines.h"

namespace QtAV {

class FboRendererPrivate;

class FboRenderer : public QObject, public VideoRenderer
{
    Q_OBJECT
    Q_DISABLE_COPY(FboRenderer)
    DPTR_DECLARE_PRIVATE(FboRenderer)
public:
    explicit FboRenderer();
    ~FboRenderer() {
        if (gl_func)
            delete gl_func;
    }

    VideoRendererId id() const Q_DECL_OVERRIDE;
    bool isSupported(VideoFormat::PixelFormat pixfmt) const Q_DECL_OVERRIDE;

    QOpenGLWidget *gl;
    QOpenGLFunctions_3_2_Core* gl_func;

public slots:
    void setResolution(QString resolution_str) {resolution = resolution_str;}
    void setPause(bool);
    void setSeek(qreal);

signals:
    void sendFeed(unsigned int texid, int width = -1, int height = -1);
    void sendFeedFbo(unsigned int fboid, int width = -1, int height = -1);
    void sendResolution(int, int);
    void updateFPS(float);
    void pause(bool);
    void seek(qreal);

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    bool receiveFrame(const VideoFrame &frame) Q_DECL_OVERRIDE;
    void drawFrame() Q_DECL_OVERRIDE;

    QString resolution;
    QElapsedTimer t;
    QList<float> fps;

    bool paused;
    qreal seek_time;
};
}

#endif //SPOUT_RENDERER_H
