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

#ifndef FEED_INPUT_H
#define FEED_INPUT_H

#include "predefines.h"
#include "DelicodeSyphonServer.h"

#ifdef _WIN32
    #include "External/Spout/Spout.h"
#endif

class FeedInput : public QObject
{
    Q_OBJECT
public:
    FeedInput(QString name, QOpenGLWidget *glwid);
    virtual ~FeedInput();

    QOpenGLWidget *gl;
    QOpenGLFramebufferObject *input_fbo;
    QOpenGLFramebufferObject *output_fbo[3];
    int output_index;

    QOpenGLFramebufferObject *fbo() {return output_fbo[output_index];}

    void start();

    void ensureFeed();
    bool receiveFeed();
    void ensureFBO();
    void capture(QString resolution);



    QString feed_name;

    unsigned int input_width, input_height;
    unsigned int output_width, output_height;


    float avg_sleeptime_ms;

    bool first_frame;
    bool error;


    float fps;
    long long framecount;

    float getSleeptime_ms();

private:
    bool feedreceiver_created;

    #ifdef _WIN32
    SpoutReceiver spoutreceiver;
    #endif

    #ifdef __APPLE__
    DelicodeSyphonClient syphonreceiver;
    #endif

    QElapsedTimer t;
};


#endif //FEED_INPUT_H
