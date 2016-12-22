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

#include "FeedOutput.h"
#include "predefines.h"

void FeedOutput::ensure(int width, int height)
{
    if(created && width == w && height == h)
        return;

    w = width;
    h = height;

    gl->makeCurrent();

    char output_name[256];
    if(out_name.isEmpty())
        strcpy(output_name, "Vidiot");
    else
        strcpy(output_name, out_name.toLocal8Bit().data());

#ifdef _WIN32
    if(created) {
        // The server will be ok after the next texture send
    }
    else {
        spoutserv.SetDX9compatible(true);
        created = spoutserv.CreateSender(output_name, w, h);

        if(created) {
            char name[256];
            spoutserv.spout.GetSpoutSenderName(name, 256);
            feedname = name;
        }
        else {
            spoutserv.SetDX9(true);
            created = spoutserv.CreateSender(output_name, w, h);

            if(created) {
                char name[256];
                spoutserv.spout.GetSpoutSenderName(name, 256);
                feedname = name;
            }
            else
                std::cout << "Error creating spout server" << std::endl;
        }
    }
#else
    GLER

    if (!syphonserver)
        syphonserver = new DelicodeSyphonServer();
    if (!syphonserver->createServer(output_name))
    {
        qDebug() << "Failed creating Syphon output";
    }
    else
    {
        if (!syphonserver->startServing())
        {
            qDebug() << "Couldn't start syphon server";
        }
        else
        {
            created = true;
            feedname = output_name;
            qDebug() << "Syphon output created " << feedname;
        }
    }

#endif

    gl->doneCurrent();
}

void FeedOutput::sendTexture(unsigned int texid, int width, int height, bool flip)
{
    if(width > 0 && height > 0)
        ensure(width, height);

    if(!created)
        return;

    GLER

    gl->makeCurrent();
#ifdef _WIN32
    spoutserv.SendTexture(texid, GL_TEXTURE_2D, width, height, flip);
#else
    // Does not work on OS X, probably due to texture rectangles being used
    //syphonserver->publishTexture(texid, width, height, flip);
#endif

    glFinish();
    gl->doneCurrent();
}

void FeedOutput::sendFBO(unsigned int fboid, int width, int height, bool flip)
{
    if(width > 0 && height > 0)
        ensure(width, height);

    if(!created)
        return;

    GLER

    gl->makeCurrent();
#ifdef _WIN32
    // spout has no fbo output?
#else
    syphonserver->publishFBO(fboid, width, height, flip);
#endif

    glFinish();
    gl->doneCurrent();
}


QStringList FeedOutput::listSources(bool list_self)
{
    QStringList list;

#ifdef WIN32
    SpoutReceiver receiver;

    int count = receiver.GetSenderCount();

    char name[256];

    for(int i=0; i<count; i++) {
        if(receiver.GetSenderName(i, name)) {
            if(name == feedname && !list_self)
                continue;

            list << QString("feed:///") + name + "?!*30 fps";
            list << QString("feed:///") + name + "?60 fps";
        }
    }
#endif

#ifdef __APPLE__
    char server_list[4096];
    DelicodeSyphonServer::getServers(server_list);
    QStringList servers = QString(server_list).split('|', QString::SkipEmptyParts);
    for (int i = 0; i < servers.count(); i++)
    {
        list << QString("feed:///") + servers.at(i) + "?!*30 fps";
        list << QString("feed:///") + servers.at(i) + "?*30 fps";
    }
#endif
    return list;
}
