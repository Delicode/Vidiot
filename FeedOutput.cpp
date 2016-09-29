#include "FeedOutput.h"

void FeedOutput::ensure(int width, int height)
{
    if(created && width == w && height == h)
        return;

    w = width;
    h = height;

    gl->makeCurrent();

#ifdef _WIN32
    if(created) {
        // The server will be ok after the next texture send
    }
    else {
        char spout_name[256];
        if(out_name.isEmpty())
            strcpy(spout_name, "Vidiot");
        else
            strcpy(spout_name, out_name.toLocal8Bit().data());

        spoutserv.SetDX9compatible(true);
        created = spoutserv.CreateSender(spout_name, w, h);

        if(created) {
            char name[256];
            spoutserv.spout.GetSpoutSenderName(name, 256);
            feedname = name;
        }
        else {
            spoutserv.SetDX9(true);
            created = spoutserv.CreateSender(spout_name, w, h);

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
    /* TODO: syphon */
#endif

    gl->doneCurrent();
}

void FeedOutput::sendTexture(unsigned int texid, int width, int height, bool flip)
{
    if(width > 0 && height > 0)
        ensure(width, height);

    if(!created)
        return;

    gl->makeCurrent();
#ifdef _WIN32
    spoutserv.SendTexture(texid, GL_TEXTURE_2D, width, height, flip);
#else
    /* TODO: syphon */
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
    // TODO: Add syphon
#endif
    return list;
}
