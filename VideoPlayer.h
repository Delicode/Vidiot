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

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "predefines.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavdevice/avdevice.h>
}

class VideoPlayer : public QObject
{
    Q_OBJECT

public:
    VideoPlayer(QString filename);
    virtual ~VideoPlayer();

    bool running;
    bool error;


    int width, height;
    float duration;
    float frame_time_s;

    unsigned char *data;

    bool playing;
    qint64 starttime;

    bool rewind;

    bool seeking;
    float seek_time_s;
    float next_seek_time_s;

    float avg_sleeptime_ms;

    QString playfile;

    QElapsedTimer t;

    QString error_str;

    QString device_name;

    float getSleeptime_ms();

public slots:
    void play();
    void showProperties();

private:
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodecContext *pCodecCtxOrig;
    int videoStream;
    AVFrame *pFrame;
    AVFrame *pFrameRGB;
    struct SwsContext *sws_ctx;

    bool readFrame();
};

#endif //VIDEOPLAYER_H
