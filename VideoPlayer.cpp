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

#include "VideoPlayer.h"

#ifdef WIN32
#include <windows.h>
#include <Dshow.h>
#include <strmif.h>
#include <dvdmedia.h>
#endif

extern "C"
{
    #include <libavfilter/avfilter.h>
    #include <libavutil/log.h>
}

#ifdef WIN32
struct dshow_ctxx {
    const AVClass *classs;

    IGraphBuilder *graph;

    char *device_name[2];
    char *device_unique_name[2];

    int video_device_number;
    int audio_device_number;

    int   list_options;
    int   list_devices;
    int   audio_buffer_size;
    int   crossbar_video_input_pin_number;
    int   crossbar_audio_input_pin_number;
    char *video_pin_name;
    char *audio_pin_name;
    int   show_video_device_dialog;
    int   show_audio_device_dialog;
    int   show_video_crossbar_connection_dialog;
    int   show_audio_crossbar_connection_dialog;
    int   show_analog_tv_tuner_dialog;
    int   show_analog_tv_tuner_audio_dialog;
    char *audio_filter_load_file;
    char *audio_filter_save_file;
    char *video_filter_load_file;
    char *video_filter_save_file;

    IBaseFilter *device_filter[2];
    IPin        *device_pin[2];
    void *capture_filter[2];
    void    *capture_pin[2];

    HANDLE mutex;
    HANDLE event[2]; /* event[0] is set by DirectShow
                      * event[1] is set by callback() */
    AVPacketList *pktl;

    int eof;

    int64_t curbufsize[2];
    unsigned int video_frame_num;

    IMediaControl *control;
    IMediaEvent *media_event;

    enum AVPixelFormat pixel_format;
    enum AVCodecID video_codec_id;
    char *framerate;

    int requested_width;
    int requested_height;
    AVRational requested_framerate;

    int sample_rate;
    int sample_size;
    int channels;
};

void showDshowProperties(IBaseFilter *device_filter, QString device_name) {
    ISpecifyPropertyPages *property_pages = NULL;
    IUnknown *device_filter_iunknown = NULL;
    HRESULT hr;
    FILTER_INFO filter_info = {0}; /* a warning on this line is false positive GCC bug 53119 AFAICT */
    CAUUID ca_guid = {0};

    hr = device_filter->QueryInterface(IID_ISpecifyPropertyPages, (void **)&property_pages);
    //hr  = IBaseFilter_QueryInterface(device_filter, &IID_ISpecifyPropertyPages, (void **)&property_pages);
    if (hr != S_OK) {
        qDebug("requested filter does not have a property page to show");
    }
    else {
        hr = device_filter->QueryFilterInfo(&filter_info);
        //hr = IBaseFilter_QueryFilterInfo(device_filter, &filter_info);
        if (hr != S_OK) {
            qDebug() << "Error 1";
        }
        else {
            hr = device_filter->QueryInterface(IID_IUnknown, (void **)&device_filter_iunknown);
            //hr = IBaseFilter_QueryInterface(device_filter, &IID_IUnknown, (void **)&device_filter_iunknown);
            if (hr != S_OK) {
                qDebug() << "Error 2";
            }
            else {
                hr = property_pages->GetPages(&ca_guid);
                //hr = ISpecifyPropertyPages_GetPages(property_pages, &ca_guid);
                if (hr != S_OK) {
                    qDebug() << "Error 3";
                }
                else {
                    wchar_t name[1024];
                    int len = device_name.toWCharArray(name);
                    name[len] = 0;

                    hr = OleCreatePropertyFrame(NULL, 0, 0, name, 1, &device_filter_iunknown, ca_guid.cElems, ca_guid.pElems, 0, 0, NULL);
                    //hr = OleCreatePropertyFrame(NULL, 0, 0, filter_info.achName, 1, &device_filter_iunknown, ca_guid.cElems, ca_guid.pElems, 0, 0, NULL);
                    if (hr != S_OK) {
                        qDebug() << "Error 4";
                    }
                }
            }
        }
    }

    if (property_pages)
        property_pages->Release();
        //ISpecifyPropertyPages_Release(property_pages);
    if (device_filter_iunknown)
        device_filter_iunknown->Release();
        //IUnknown_Release(device_filter_iunknown);
    if (filter_info.pGraph)
        filter_info.pGraph->Release();
        //IFilterGraph_Release(filter_info.pGraph);
    if (ca_guid.pElems)
        CoTaskMemFree(ca_guid.pElems);
}

#endif

VideoPlayer::VideoPlayer(QString filename) : QObject(NULL),
    pFormatCtx(NULL),
    pCodecCtx(NULL),
    pCodecCtxOrig(NULL),
    videoStream(-1),
    pFrame(NULL),
    pFrameRGB(NULL),
    sws_ctx(NULL),
    running(false),
    error(false),
    width(0),
    height(0),
    duration(-1.f),
    data(NULL),
    playfile(filename),
    playing(true),
    starttime(0),
    seeking(false),
    seek_time_s(0),
    next_seek_time_s(-1),
    avg_sleeptime_ms(0),
    rewind(false)
{
    qDebug() << "Registering av all";
    av_register_all();

    QStringList source = filename.split(":///");

    QString device = source.first();
    filename = source.last();

    if(device == "dshow" || device == "gdigrab") {

        avdevice_register_all();

        pFormatCtx = avformat_alloc_context();
        AVInputFormat *iformat = av_find_input_format(device.toLocal8Bit().constData());

        if(!iformat) {
            error_str = "Couldn't open DirectShow format.";
            error = true;
            return;
        }

        int w = -1;
        int h = -1;
        int f = -1;
        QString codec = "raw";

        if(filename.indexOf("?")) {
            QStringList optionslist = filename.split("?");
            filename = optionslist.first();
            QString options = optionslist.last();
            options.remove("*");
            options.remove("!");

            QString width_str = options.split(" x ").first();
            QString height_str = options.split(" @ ").first().split(" x ").last();
            QString fps_str = options.split(" @ ").last().split(" fps ").first();
            codec = options.split("(").last().replace(")", "");

            bool is_int = false;
            int _w = width_str.toInt(&is_int);
            if(is_int) w = _w;
            int _h = height_str.toInt(&is_int);
            if(is_int) h = _h;
            int _f = fps_str.toInt(&is_int);
            if(is_int) f = _f;
        }

        AVDictionary* options = NULL;

        if(device == "gdigrab") {
            av_dict_set(&options,"offset_x","20",0);
            av_dict_set(&options,"offset_y","40",0);
            av_dict_set(&options,"video_size","640x480",0);
        }
        else {
            if(w > 0 && h > 0)
                av_dict_set(&options, "video_size", QString("%1x%2").arg(w).arg(h).toLocal8Bit().data(), 0);
            //av_dict_set(&options, "vcodec", "mjpeg", NULL);
            if(f > 0)
                av_dict_set(&options, "framerate", QString::number(f).toLocal8Bit().data(), 0);

            if(codec == "mjpeg") {
                pFormatCtx->video_codec_id = AV_CODEC_ID_MJPEG;
                av_format_set_video_codec(pFormatCtx, avcodec_find_decoder(AV_CODEC_ID_MJPEG));
            }
            else if(codec == "dvvideo") {
                pFormatCtx->video_codec_id = AV_CODEC_ID_DVVIDEO;
                av_format_set_video_codec(pFormatCtx, avcodec_find_decoder(AV_CODEC_ID_DVVIDEO));
            }

            device_name = filename;

            filename = "video=" + filename;
        }

        if(avformat_open_input(&pFormatCtx, filename.toLocal8Bit().constData(), iformat, &options) != 0) {
            error_str = "Couldn't open device: " + filename;
            error = true;
            return;
        }
    }
    else {
        if(avformat_open_input(&pFormatCtx, filename.toLocal8Bit().constData(), NULL, NULL) != 0) {
            error_str = "Couldn't open file: " + filename;
            error = true;
            return;
        }
    }

    if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        error_str = "Couldn't find stream info";
        error = true;
        return;
    }

    videoStream = -1;
    for(int i=0; pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if(videoStream == -1) {
        error_str = "Couldn't find a video stream";
        error = true;
        return;
    }

    pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;

    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if(pCodec == NULL) {
        error_str = "Couldn't find a decoder";
        error = true;
        return;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
        error_str = "Couldn't copy codec context";
        error = true;
        return;
    }

    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
        error_str = "Couldn't open codec context";
        error = true;
        return;
    }

    if(pFormatCtx->duration != AV_NOPTS_VALUE)
        duration = (double)pFormatCtx->duration / (double)AV_TIME_BASE;

    width = pCodecCtx->width + (pCodecCtx->width%2);
    height = pCodecCtx->height + (pCodecCtx->height%2);

    data = new unsigned char[3*width*height];
    memset(data, 0, 3*width*height*sizeof(unsigned char));

    pFrame = av_frame_alloc();

    pFrameRGB = av_frame_alloc();

    uint8_t *buffer = NULL;
    int numBytes;

    numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, width, height);
    buffer = (uint8_t*)av_malloc(numBytes*sizeof(uint8_t));

    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, width, height);

    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, width, height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);

    qDebug() << "Bits per coded sample:" << pCodecCtx->bits_per_coded_sample;
    qDebug() << "Codec name:" << pCodecCtx->codec_name;
    qDebug() << "Codec format:" << pCodecCtx->pix_fmt;
    qDebug() << "Resolution:" << width << "x" << height;
    qDebug() << "Framerate:" << pCodecCtx->framerate.den << "/" << pCodecCtx->framerate.num << av_q2d(pCodecCtx->framerate);
    qDebug() << "Start times:" << pFormatCtx->start_time << pFormatCtx->start_time_realtime;
    qDebug() << "Framebase:" << pFormatCtx->streams[videoStream]->time_base.den << "/" << pFormatCtx->streams[videoStream]->time_base.num << av_q2d(pFormatCtx->streams[videoStream]->time_base);

    t.start();
}

VideoPlayer::~VideoPlayer()
{
    if(data)
        delete [] data;

    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avcodec_close(pCodecCtx);
    avcodec_close(pCodecCtxOrig);
    avformat_close_input(&pFormatCtx);
}

void VideoPlayer::showProperties()
{
    if(playfile.startsWith("dshow:///")) {
#ifdef WIN32
        /* the dshow_ctxx struct is just copied from somewhere inside ffmpeg (dshow_ctx in the source).. if the struct changes radically things could get ugly */
        dshow_ctxx *ctx = (dshow_ctxx*)pFormatCtx->priv_data;
        QtConcurrent::run(&showDshowProperties, ctx->device_filter[0], device_name);
#endif
    }
}

void VideoPlayer::play()
{
    if(seeking) {
        seek_time_s = next_seek_time_s;
        next_seek_time_s = -1;

        if(seek_time_s >= 0) {
            AVRational seek_timebase;
            seek_timebase.num = 1;
            seek_timebase.den = 1000;
            int seek_time = av_rescale_q(1000*seek_time_s, seek_timebase, pFormatCtx->streams[videoStream]->time_base);
            if(av_seek_frame(pFormatCtx, videoStream, seek_time, AVSEEK_FLAG_BACKWARD) < 0) {
                if(av_seek_frame(pFormatCtx, videoStream, seek_time, AVSEEK_FLAG_ANY) < 0) {
                    qDebug() << "Couldn't seek";
                    return;
                }
            }

            avcodec_flush_buffers(pCodecCtx);

            readFrame();
            int tries_left = 500;
            while(frame_time_s < seek_time_s && tries_left) {
                qDebug() << "Seek" << tries_left << "-" << seek_time_s << "currently at:" << frame_time_s << "distance:" << seek_time_s - frame_time_s;
                readFrame();
                tries_left--;
            }
        }
    }
    else if(rewind || !readFrame()) {
        rewind = false;

        //qDebug() << "seeking to beginning";

        if(av_seek_frame(pFormatCtx, videoStream, 0, AVSEEK_FLAG_ANY) < 0) {
            qDebug() << "Couldn't seek";
            return;
        }

        avcodec_flush_buffers(pCodecCtx);

        t.start();
        starttime = 0;

        readFrame();
    }
}

bool VideoPlayer::readFrame()
{
    int frameFinished = 0;
    AVPacket packet;

    QElapsedTimer tt; tt.start();

    int packets = 0;

    long long read_ns = 0;

    while(av_read_frame(pFormatCtx, &packet) >= 0) {

        read_ns = tt.nsecsElapsed();

        packets++;

        if(packet.stream_index==videoStream) {
            long long decode_ns = 0;
            long long scale_ns = 0;
            QElapsedTimer et;
            et.start();

            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            decode_ns = et.nsecsElapsed(); et.start();

            if(frameFinished) {
                frame_time_s = (float)av_frame_get_best_effort_timestamp(pFrame) * (float)av_q2d(pFormatCtx->streams[videoStream]->time_base);

                if(seeking) {
                    if(frame_time_s < seek_time_s) {
                        /* this is not the frame we want, so don't waste time converting the frame */
                        av_free_packet(&packet);
                        break;
                    }
                }

                if(!playfile.startsWith("dshow:///") && !playfile.startsWith(""))
                    std::cout << "playpos:" << frame_time_s << std::endl;

                et.start();

                sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                scale_ns = et.nsecsElapsed(); et.start();

                if(pFrameRGB->linesize[0] != 3*width) {
                    int copy_width = 3*width < pFrameRGB->linesize[0] ? 3*width : pFrameRGB->linesize[0];
                    memset(data, 0, width*height*3*sizeof(unsigned char));
                    for(int i=0; i<height; i++)
                        memcpy(data+i*3*width, pFrameRGB->data[0]+i*pFrameRGB->linesize[0], copy_width*sizeof(unsigned char));
                }
                else {
                    memcpy(data, pFrameRGB->data[0], width*height*3*sizeof(unsigned char));
                }

                av_free_packet(&packet);

                //qDebug() << "Frame processing:" << read_ns/1000000 << decode_ns/1000000 << scale_ns/1000000 << et.nsecsElapsed()/1000000;
                break;
            }
        }
        av_free_packet(&packet);
    }

    //qDebug() << "Total update:" << tt.nsecsElapsed()/1000000 << packets << "packets read";

    return frameFinished != 0;
}

float VideoPlayer::getSleeptime_ms()
{
    if(playfile.startsWith("dshow:///")) {
        float sleeptime_s = 0.033333f - (float)t.nsecsElapsed()/1000000000.f;
        t.restart();
        return 1000.f*sleeptime_s;
    }
    float elapsed_time_s = (float)(starttime+t.nsecsElapsed())/1000000000.f;
    float sleeptime_ms = 1000.f*(frame_time_s-elapsed_time_s);

    /* keep a running average of how much the stream is behind real time */
    if(fabsf(avg_sleeptime_ms-sleeptime_ms) > 30) {
        avg_sleeptime_ms = sleeptime_ms;
    }
    else {
        float fac = 0.05f;
        avg_sleeptime_ms = (1.f-fac) * avg_sleeptime_ms + fac*sleeptime_ms;
    }

    return sleeptime_ms;
}
