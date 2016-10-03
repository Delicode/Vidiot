#include "DelicodeSyphonServer.h"
#include "QImage.h"
#import "Lib/Syphon.framework/Headers/Syphon.h"
#import <OpenGL/CGLMacro.h>

#include <qdebug.h>

DelicodeSyphonServer::DelicodeSyphonServer()
{
	mSyphon = nil;
	has_clients = false;
	recreated = false;
	hosting_name = "";
	memset(server_name, '\0', 2048*sizeof(unsigned char));
	has_quit = false;
	context_is_valid = false;
	has_started = false;
}

DelicodeSyphonServer::~DelicodeSyphonServer()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	if(mSyphon) {
		[(SyphonServer *)mSyphon stop];
		[(SyphonServer *)mSyphon release];
	}
	[pool drain];
}

bool DelicodeSyphonServer::createServer(char *name)
{
    sprintf(server_name, "%s", name);
    GLenum err;

	NSString* title = [NSString stringWithCString:server_name];
	if (!mSyphon)
	{
		recreated = true;

        CGLContextObj cgl_ctx = CGLGetCurrentContext();
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            qDebug() << "Syphon create server: error getting OpenGL context: " << err;
        }

        if (cgl_ctx == NULL)
		{
			context_is_valid = false;
			return false;
		}
		mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            qDebug() << "Syphon create server: OpenGL error initting: " << err;
        }

		hosting_name = ([title UTF8String]);
		if ([(SyphonServer *)mSyphon bindToDrawFrameOfSize:(NSSize){512, 256}] == YES)
			[(SyphonServer *)mSyphon unbindAndPublish];
		[(SyphonServer *)mSyphon setName:title];
		context_is_valid = true;
		has_started = true;
	}
	else
		recreated = false;

	if (mSyphon == nil)
		return false;
	return true;
}

bool DelicodeSyphonServer::publishScreen()
{
	CGLContextObj cgl_ctx = CGLGetCurrentContext();
	GLint dims[4];

	if (cgl_ctx == NULL)
	{
		context_is_valid = false;
		return false;
	}
	context_is_valid = true;

	glPushAttrib(GL_TEXTURE_BIT);
	glGetIntegerv(GL_VIEWPORT, dims);

	if (!mSyphon)
	{
		CGFloat dims_x = dims[2];
		CGFloat dims_y = dims[3];
		NSString* title = [NSString stringWithCString:server_name];
		mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];
		[(SyphonServer *)mSyphon bindToDrawFrameOfSize:(NSSize){dims_x, dims_y}];
		[(SyphonServer *)mSyphon unbindAndPublish];
		recreated = true;
	}
	else
		recreated = false;

	SyphonImage* img = [(SyphonServer *)mSyphon newFrameImage];
	if(img) {
		glReadBuffer(GL_FRONT);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, [img textureName]);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, dims[0], dims[1], dims[2], dims[3]);

		CGFloat dims_x = dims[2];
		CGFloat dims_y = dims[3];

		[(SyphonServer *)mSyphon bindToDrawFrameOfSize:(NSSize){dims_x,dims_y}];
		[(SyphonServer *)mSyphon unbindAndPublish];

		[img release];
		glPopAttrib();
		return true;
	}
	else {
		glPopAttrib();
		[(SyphonServer *)mSyphon stop];
		[(SyphonServer *)mSyphon release];
		mSyphon = nil;
		has_quit = true;
		return false;
	}
 }

bool DelicodeSyphonServer::bindFBO(int res_x, int res_y)
{
	if (!mSyphon)
	{
		return false;
	}

	SyphonServer* s = (SyphonServer *)mSyphon;
	CGFloat dims_x = res_x;
	CGFloat dims_y = res_y;

	if ([s bindToDrawFrameOfSize:(NSSize){dims_x, dims_y}] == NO)
		return false;
	return true;
}

bool DelicodeSyphonServer::unbindFBO()
{
	if (!mSyphon)
	{
		return false;
	}

	SyphonServer* s = (SyphonServer *)mSyphon;
	[s unbindAndPublish];
	return true;
}

bool DelicodeSyphonServer::publishFBO(unsigned int id, int res_x, int res_y, bool flip)
{
	if(id)
	{
		CGLContextObj cgl_ctx = CGLGetCurrentContext();
		if (cgl_ctx == NULL)
		{
			context_is_valid = false;
			return false;
		}

		if (!mSyphon)
		{
			const char *name = "Render";
			context_is_valid = true;

			NSString* title = [NSString stringWithCString:name];
			mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];
			if (!mSyphon)
			{
				return false;
			}
		}

		NSSize res;
		res.width = 1024;
		res.height = 1024;

		GLenum err = glGetError();

		SyphonServer* serv = (SyphonServer *)mSyphon;

		int retcode = 0;
		[serv delicode_publishFBO:NSMakeSize(res_x,res_y) fbo_id:id];
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			qDebug() << "error after publish fbo: " << err;
		}
	}
	else
	{
		[(SyphonServer *)mSyphon stop];
		[(SyphonServer *)mSyphon release];
		mSyphon = nil;
		return false;
	}
	return true;
}

bool DelicodeSyphonServer::startServing()
{
    GLenum err;

    NSString* title = [NSString stringWithCString:server_name];
    if (!mSyphon)
    {
        recreated = true;
        CGLContextObj cgl_ctx = CGLGetCurrentContext();

        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            qDebug() << "Syphon start serving: OpenGL error getting context: " << err;
        }

        if (cgl_ctx == NULL)
        {
            context_is_valid = false;
            return false;
        }
        mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];

        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            qDebug() << "Syphon start serving: OpenGL error initting: " << err;
        }

        hosting_name = ([title UTF8String]);
        if ([(SyphonServer *)mSyphon bindToDrawFrameOfSize:(NSSize){512, 256}] == YES)
            [(SyphonServer *)mSyphon unbindAndPublish];
        [(SyphonServer *)mSyphon setName:title];
        context_is_valid = true;
    }
    else
        recreated = false;

    if (mSyphon == nil)
        return false;
    return true;
}

bool DelicodeSyphonServer::publishTexture(unsigned int id, int res_x, int res_y, bool flip)
{
    GLenum err;

	if (!has_started)
		startServing();
	CGLContextObj cgl_ctx = CGLGetCurrentContext();

    err = glGetError();
    if (err != GL_NO_ERROR)
    {
        qDebug() << "Syphon publish texture: error in opengl get context: " << err;
    }

	if (cgl_ctx == NULL)
	{
		context_is_valid = false;
        qDebug() << "Syphon publish texture: opengl context is invalid";
		return false;
	}

    err = glGetError();
	if (err != GL_NO_ERROR)
	{
        qDebug() << "syphon publish texture error: " << err;
	}

	if(id) {
		if (!mSyphon)
		{
			recreated = true;
			context_is_valid = true;

			NSString* title = [NSString stringWithCString:server_name];
			mSyphon = [[SyphonServer alloc] initWithName:title context:CGLGetCurrentContext() options:nil];
            err = glGetError();
            if (err != GL_NO_ERROR)
            {
                qDebug() << "Syphon publish texture: OpenGL error initting: " << err;
            }
		}
		else
			recreated = false;

		err = glGetError();
		if (err != GL_NO_ERROR)
		{
            qDebug() << "Syphon publish texture: error before " << err;
		}
		SyphonServer* s = (SyphonServer *)mSyphon;
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
            qDebug() << "Syphon publish texture: error after getting syphon " << err;
		}

		//if ([s bindToDrawFrameOfSize:(NSSize){512, 256}] == YES)
		//	[s unBindAndPublish];

		test_name = [s.name UTF8String];
		[s publishFrameTexture:id textureTarget:GL_TEXTURE_2D imageRegion:NSMakeRect(0,0,res_x,res_y) textureDimensions:NSMakeSize(res_x,res_y) flipped:flip];
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
            qDebug() << "Syphon publish texture: error after publish: " << err;
		}

		if (s.hasClients == NO)
			has_clients = false;
		else
			has_clients = true;
		return true;
	}
	else {
		[(SyphonServer *)mSyphon stop];
		[(SyphonServer *)mSyphon release];
		mSyphon = nil;
		has_quit = true;
		return false;
	}
}

void DelicodeSyphonServer::getServers(char *name_list)
{
	NSArray *server_list = [[SyphonServerDirectory sharedDirectory] servers];

	sprintf(name_list, "");

	int unknown = 1;
	for(NSDictionary *server in server_list) {
		NSString* app_name = [server objectForKey:SyphonServerDescriptionAppNameKey];
		NSString* server_name = [server objectForKey:SyphonServerDescriptionNameKey];

		const char *orig_app_str = [app_name UTF8String];
		const char *orig_server_str = [server_name UTF8String];

		char app_str[1024]; strcpy(app_str, orig_app_str);
		char server_str[1024]; strcpy(server_str, orig_server_str);

		for(int i=0; i<strlen(app_str); i++) {
			if(app_str[i] == '|')
				app_str[i] = '.';
		}

		for(int i=0; i<strlen(server_str); i++) {
			if(server_str[i] == '|')
				server_str[i] = '.';
		}

        qDebug() << "found server: " << server_str << ", app str: " << app_str;

		if(strlen(app_str) == 0 && strlen(server_str) == 0)
			sprintf(name_list + strlen(name_list), "Unknown application %i|", unknown++);
		else if(strlen(server_str) == 0)
			sprintf(name_list + strlen(name_list), "%s|", app_str);
		else
			sprintf(name_list + strlen(name_list), "%s - %s|", app_str, server_str);
	}
}

DelicodeSyphonClient::DelicodeSyphonClient(const char* name)
{
	mSyphon = nil;
	mFrame = nil;
	texID = 0;
	width = 1;
	height = 1;
	number_of_servers = 0;
	mFbo = 0;

    if (name != NULL && strlen(name) > 0)
        strcpy(server_name, name);
	//qDebug() << "Will search for server named " << QString::fromStdString(std::string(server_name));

	number_of_servers = 0;
	isConnected = false;
}

void DelicodeSyphonClient::setName(const char* name)
{
    strcpy(server_name, name);
}

bool DelicodeSyphonClient::connectToServer()
{
    NSArray *server_list = [[SyphonServerDirectory sharedDirectory] servers];

    for(NSDictionary *server in server_list) {
        NSString* sname = [server objectForKey:SyphonServerDescriptionAppNameKey];

        const char *orig_server_str = [sname UTF8String];

        char server_str[1024]; strcpy(server_str, orig_server_str);

        if (strcmp(server_str, server_name) == 0)
        {
            mSyphon = [[SyphonClient alloc] initWithServerDescription:server options:nil newFrameHandler:nil];
            if ( ((SyphonClient *)mSyphon).isValid == NO)
            {
                qDebug() << "Syphon connect to server: Could not connect to server: " << server_name;
                isConnected = false;
                return false;
            }
            qDebug() << "Syphon connect to server: Connected to server successfully: " << server_name;
            SyphonClient* c;
            c = ((SyphonClient *)mSyphon);
            NSString* temp1 = c.serverDescription[SyphonServerDescriptionUUIDKey];
            std::string temp2 = [temp1 UTF8String];
            //qDebug() << "Connected server uuid is: " << temp2.c_str();

            isConnected = true;
            return true;
        }
        else
        {
            qDebug() << "server name not matching: " << server_str << " vs " << server_name;
        }
    }

    qDebug() << "no server found with name: " << server_name;

    if (mSyphon != nil)
        [(SyphonClient *)mSyphon release];
    return false;
}

void DelicodeSyphonClient::getServers()
{
	number_of_servers = 0;

	NSArray* server_list = [[SyphonServerDirectory sharedDirectory] servers];
	NSString* search_name_nsstring = [NSString stringWithCString:server_name];

	std::string cppString([search_name_nsstring UTF8String]);

	for(NSDictionary *server in server_list) {
		NSString* app_name = [server objectForKey:SyphonServerDescriptionAppNameKey];
		NSString* srv_name = [server objectForKey:SyphonServerDescriptionNameKey];

		number_of_servers++;

		if ([srv_name rangeOfString:search_name_nsstring].location != NSNotFound)
		{
			std::string c([srv_name UTF8String]);
			//qDebug() << "Found server with proper name: " << QString::fromStdString(c);
			//sprintf(assigned_server, "%s", &c[0]);
		}
		else
		{
			std::string c([srv_name UTF8String]);
			//qDebug() << "Server " << QString::fromStdString(c) << " did not contain substr " << QString::fromStdString(std::string(cppString));
		}
	}
}

void DelicodeSyphonClient::printServers()
{
	NSArray *server_list = [[SyphonServerDirectory sharedDirectory] servers];
	NSString* search_name_nsstring = [NSString stringWithCString:server_name];

	int count = 0;

	for(NSDictionary *server in server_list) {
		NSString* app_name = [server objectForKey:SyphonServerDescriptionAppNameKey];
		NSString* server_name = [server objectForKey:SyphonServerDescriptionNameKey];

		std::string out([server_name UTF8String]);
		count++;
		//qDebug() << "Server #" << count << ": " << QString::fromStdString(out);

		if ([server_name rangeOfString:search_name_nsstring].location == NSNotFound)
		{
		}
		else
		{
			std::string c([server_name UTF8String]);
			//qDebug() << "Found server with proper name: " << QString::fromStdString(c);
		}
	}
}

void DelicodeSyphonClient::free_frame()
{
	if (mFrame)
	{
		SyphonImage* i = (SyphonImage *)mFrame;
		[i release];
		mFrame = nil;
	}
}

bool DelicodeSyphonClient::update(unsigned int& input_width, unsigned int& input_height)
{
	if(!mSyphon) {
		texID = 0;
		width = 1;
		height = 1;
        qDebug() << "Syphon update: syphon not initialized";
        return false;
	}

	CGLContextObj cgl_ctx = CGLGetCurrentContext();

	if (cgl_ctx == NULL)
	{
        qDebug() << "Syphon update: Context is not valid";
        return false;
	}
	GLenum err = glGetError();

	SyphonClient* c = (SyphonClient *)mSyphon;

	if (c.isValid == NO)
	{
		isConnected = false;
        qDebug() << "Syphon update: no connection to server";
        return false;
	}
	isConnected = true;
	NSString* temp1 = c.serverDescription[SyphonServerDescriptionUUIDKey];
	std::string temp2 = [temp1 UTF8String];
    //qDebug() << "Connected server uuid is: " << temp2.c_str();
	temp1 = c.serverDescription[SyphonServerDescriptionNameKey];
	temp2 = [temp1 UTF8String];
	//qDebug() << "Connected server name is: " << QString::fromStdString(temp2);
	temp1 = c.serverDescription[SyphonServerDescriptionAppNameKey];
	temp2 = [temp1 UTF8String];
	//qDebug() << "Connected server app name is: " << QString::fromStdString(temp2);

	// Clear error stack
	err = glGetError();
	if (c.hasNewFrame == YES) {
		mFrame = [(SyphonClient *)mSyphon newFrameImageForContext:cgl_ctx];
		err = glGetError();
		if (err != GL_NO_ERROR)
		{
			texID = 0;
            qDebug() << "Syphon update: error getting frame: " << err;
            return false;
		}
    }
	else {
        qDebug() << "Syphon update: no new frames";
        return false;
    }

	if(!mFrame) {
        texID = 0;
		width = 1;
		height = 1;
        qDebug() << "Syphon update: frame is null";
        return false;
    }

	texID = ((SyphonImage*)mFrame).textureName;
	width = ((SyphonImage*)mFrame).textureSize.width;
	height = ((SyphonImage*)mFrame).textureSize.height;
    input_width = width;
    input_height = height;

	if(!mFbo)
		glGenFramebuffersEXT(1, &mFbo);
	err = glGetError();
	if (err != GL_NO_ERROR)
        qDebug() << "Syphon update:  gen framebuffersext " << err;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);
	err = glGetError();
	if (err != GL_NO_ERROR)
        qDebug() << "Syphon update:  bindframebuffersext " << err;


	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, texID, 0);
	err = glGetError();
	if (err != GL_NO_ERROR)
        qDebug() << "Syphon update:  framebuffertex2d " << err;


	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	err = glGetError();
	if (err != GL_NO_ERROR)
        qDebug() << "Syphon update: error bindframebufferext " << err;

    return true;
}

bool DelicodeSyphonClient::copyToTexture(unsigned int texid)
{
	CGLContextObj cgl_ctx = CGLGetCurrentContext();
    if (cgl_ctx == nil)
    {
        qDebug() << "Syphon copy texture: OpenGL context is null";
        return false;
    }

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);
	glBindTexture(GL_TEXTURE_2D, texid);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glReadBuffer(GL_BACK);
    return true;
}

void DelicodeSyphonClient::release()
{
	if(!mSyphon && !mFrame)
		return;

	[(SyphonImage *)mFrame release];

	mFrame = nil;

	if (mSyphon)
		[(SyphonClient *)mSyphon release];
	mSyphon = nil;
}

DelicodeSyphonClient::~DelicodeSyphonClient()
{
	if(mFrame)
		[(SyphonImage *)mFrame release];
	if(mSyphon)
		[(SyphonClient *)mSyphon stop];
}
