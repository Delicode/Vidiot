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

#ifndef DELICODE_SYPHON_SERVER
#define DELICODE_SYPHON_SERVER

#include "predefines.h"

class DelicodeSyphonServer {
public:
    DelicodeSyphonServer();
    ~DelicodeSyphonServer();

    bool createServer (char *name);
    bool publishScreen();
    bool publishFBO(unsigned int id, int res_x, int res_y, bool flip);
    bool publishTexture(unsigned int id, int res_x = 640, int res_y = 480, bool flip = true);
    bool startServing();
    bool bindFBO(int res_x = 512, int res_y = 256);
    bool unbindFBO();

    static void getServers(char *server_list);
    char server_name[2048];
    bool has_clients;
    bool recreated;
    bool has_quit;
    bool context_is_valid;
    std::string hosting_name;
    std::string test_name;
    bool has_started;
protected:
    void *mSyphon;
};

class DelicodeSyphonClient {
public:
    DelicodeSyphonClient(const char* name = NULL);
    ~DelicodeSyphonClient();
    unsigned int texID;
    unsigned int width;
    unsigned int height;
    char server_name[1024];
    const int MAX_SERVER_NAMES = 32;

    int number_of_servers;
    bool update(unsigned int& input_width, unsigned int& input_height);
    void release();
    void getServers();
    bool connectToServer();
    void setName(const char* name);
    void printServers();
    void free_frame();
    bool isConnected;
    bool copyToTexture(unsigned int texid);
protected:
    void *mSyphon;
    void *mFrame;
    unsigned int mFbo;
};

#endif //DELICODE_SYPHON_SERVER
