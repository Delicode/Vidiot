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

#include "FeedInput.h"

FeedInput::FeedInput(QString name, QOpenGLWidget *glwid) : QObject(NULL),
    feedreceiver_created(false),
    framecount(0),
    fps(30.f),
    error(false),
    gl(glwid),
    input_fbo(NULL),
    output_index(0),
    input_width(0),
    input_height(0),
    output_width(0),
    output_height(0),
    avg_sleeptime_ms(0)
{


    for(int i=0; i<3; i++)
        output_fbo[i] = NULL;

    if(name.contains("?")) {
        feed_name = name.split("?").first();
        QString fps_str = name.split("?").last();
        fps_str.remove("!");
        fps_str.remove("*");
        fps_str.remove(" fps");

        bool is_float = false;
        float f = fps_str.toFloat(&is_float);
        if(is_float)
            fps = f;
    }
    else {
        feed_name = name;
    }
    setObjectName("FeedInput");
}



FeedInput::~FeedInput()
{
    if(feedreceiver_created)
    {
#ifdef WIN32
        spoutreceiver.ReleaseReceiver();
#endif

#ifdef __APPLE__
        syphonreceiver.release();
#endif
    }
}

void FeedInput::ensureFeed()
{
    if(!feedreceiver_created) {
        gl->makeCurrent();

        char name[256];
        sprintf(name, feed_name.toLocal8Bit().data());

#ifdef _WIN32

        feedreceiver_created = spoutreceiver.CreateReceiver(name, input_width, input_height);

        if(feedreceiver_created) {
            qDebug() << "Spout server created";
        }
        else {
            spoutreceiver.SetDX9(true);
            feedreceiver_created = spoutreceiver.CreateReceiver(name, input_width, input_height);

            if(feedreceiver_created)
                qDebug() << "Spout receiver initialized in DX9 mode.";
            else
                qDebug() << "Spout receiver couldn't be initialized.";
        }
#else
        syphonreceiver.setName(name);
        feedreceiver_created = syphonreceiver.connectToServer();

        if (feedreceiver_created == false)
        {
            qDebug() << "Failed starting Syphon receiver: " << name;
        }
#endif
        gl->doneCurrent();
    }
}

bool FeedInput::receiveFeed()
{
#ifdef _WIN32
    char name[256];
    sprintf(name, feed_name.toLocal8Bit().data());
    return spoutreceiver.ReceiveTexture(name, input_width, input_height, input_fbo->texture(), GL_TEXTURE_2D, true);
#else
    if (!syphonreceiver.update(input_width, input_height))
        return false;
    ensureFBO();
    syphonreceiver.copyToTexture(input_fbo->texture());
    return true;
#endif
}

void FeedInput::ensureFBO()
{
    if(input_fbo == NULL || input_fbo->width() != input_width || input_fbo->height() != input_height) {

        if(input_fbo)
            delete input_fbo;

        input_fbo = new QOpenGLFramebufferObject(input_width, input_height, GL_TEXTURE_2D);

        qDebug() << "Created input fbo" << input_fbo->width() << "x" << input_fbo->height();
    }

    if(output_fbo[output_index] == NULL || output_fbo[output_index]->width() != output_width || output_fbo[output_index]->height() != output_height) {

        if(output_fbo[output_index])
            delete output_fbo[output_index];

        output_fbo[output_index] = new QOpenGLFramebufferObject(output_width, output_height, GL_TEXTURE_2D);

        qDebug() << "Created output fbo" << output_fbo[output_index]->width() << "x" << output_fbo[output_index]->height();

    }
}



void FeedInput::capture(QString resolution)
{
    if(!feedreceiver_created) {
        ensureFeed();

        if(!feedreceiver_created) {
            error = true;
            return;
        }

        t.start();
    }

    output_width = input_width - (input_width % 2);
    output_height = input_height - (input_height % 2);

    if(resolution.startsWith("50%")) {
        output_width = output_width/2;
        output_height = output_height/2;
    }
    else if(resolution.startsWith("25%")) {
        output_width = output_width/4;
        output_height = output_height/4;
    }
    else if(resolution.startsWith("1080p")) {
        output_width = 1920;
        output_height = 1080;
    }
    else if(resolution.startsWith("720p")) {
        output_width = 1280;
        output_height = 720;
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
                output_width = tw - (tw % 2);
                output_height = th - (th % 2);
            }
        }
    }

    framecount++;

    if(error)
        return;

    output_index = (output_index+1)%3;


    gl->makeCurrent();
#ifdef WIN32
    ensureFBO();
#endif

    if(!receiveFeed()) {
        qDebug() << "Error receiving texture";
        gl->doneCurrent();
        return;
    }

    if (input_fbo->width() == 0 || input_fbo->height() == 0)
        return;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

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

    output_fbo[output_index]->bind();
    output_fbo[output_index]->blitFramebuffer(output_fbo[output_index], QRect(x, y, sw, sh), input_fbo, QRect(0, 0, input_fbo->width(), input_fbo->height()), GL_COLOR_BUFFER_BIT, GL_LINEAR);
    output_fbo[output_index]->release();

    glFinish();

    gl->doneCurrent();
}

float FeedInput::getSleeptime_ms()
{
    float frame_time_s = (float)framecount/fps;
    float elapsed_time_s = (float)t.nsecsElapsed()/1000000000.f;

    float sleeptime_ms = 1000.f*(frame_time_s-elapsed_time_s);
    if(fabsf(avg_sleeptime_ms-sleeptime_ms) > 30) {
        avg_sleeptime_ms = sleeptime_ms;
    }
    else {
        float fac = 0.05f;
        avg_sleeptime_ms = (1.f-fac) * avg_sleeptime_ms + fac*sleeptime_ms;
    }

    return sleeptime_ms;
}
