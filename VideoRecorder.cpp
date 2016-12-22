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

#include "VideoRecorder.h"

#define SET_ERROR_RETURN(description) { qDebug() << description; error = true; return; }
#define SET_ERROR_RETURN_NULL(description) { qDebug() << description; error = true; return NULL; }
#define SET_ERROR_RETURN_FALSE(description) { qDebug() << description; error = true; return false; }

AVCodecID VideoRecorder::codec_h264_qsv = AV_CODEC_ID_NONE;
AVCodecID VideoRecorder::codec_prores = AV_CODEC_ID_NONE;
AVCodecID VideoRecorder::codec_libvpxvp9 = AV_CODEC_ID_NONE;
AVCodecID VideoRecorder::codec_libx264 = AV_CODEC_ID_NONE;
AVCodecID VideoRecorder::codec_nvenc_h264 = AV_CODEC_ID_NONE;
AVCodecID VideoRecorder::codec_libtheora = AV_CODEC_ID_NONE;

QStringList VideoRecorder::list_codecs(bool log)
{
    av_register_all();

    avdevice_register_all();


    QMap<int, QString> codecs;

    if(log)
        qDebug() << "----- Encoders -----";

    AVCodec * codec = av_codec_next(NULL);
    while(codec != NULL)
    {
        if(codec->type == AVMEDIA_TYPE_VIDEO) {
            if(av_codec_is_encoder(codec)) {
                if(log) {
                    codecs.insert(codec->id, QString("%1 (%2)").arg(codec->long_name).arg(codec->name));
                    qDebug() << codec->id << codec->long_name << codec->name;
                }

                if(strcmp(codec->name, "h264_qsv") == 0)
                    codec_h264_qsv = codec->id;
                else if(strcmp(codec->name, "prores") == 0)
                    codec_prores = codec->id;
                else if(strcmp(codec->name, "libvpx-vp9") == 0)
                    codec_libvpxvp9 = codec->id;
                else if(strcmp(codec->name, "libx264") == 0)
                    codec_libx264 = codec->id;
                else if(strcmp(codec->name, "nvenc_h264") == 0)
                    codec_nvenc_h264 = codec->id;
                else if(strcmp(codec->name, "libtheora") == 0)
                    codec_libtheora = codec->id;
            }
        }
        codec = av_codec_next(codec);
    }

    if(log) {
        qDebug() << "";
        qDebug() << "";

        qDebug() << "h264_qsv" << codec_h264_qsv;
        qDebug() << "prores" << codec_prores;
        qDebug() << "libvpx-vp9" << codec_libvpxvp9;
        qDebug() << "libx264" << codec_libx264;
        qDebug() << "nvenc_h264" << codec_nvenc_h264;
        qDebug() << "libtheora" << codec_libtheora;

        qDebug() << "";
        qDebug() << "";
        qDebug() << "----- Containers -----";

        AVOutputFormat * oformat = av_oformat_next(NULL);
        while(oformat != NULL)
        {
            qDebug() << oformat->long_name << "-" << oformat->name;

            if(codecs.contains(oformat->video_codec))
                qDebug() << "     default:" << codecs[oformat->video_codec];
            else
                qDebug() << "     default:" << "unknown";

            if (oformat->codec_tag != NULL)
            {
                int i = 0;

                AVCodecID cid = AV_CODEC_ID_MPEG1VIDEO;
                while (cid != AV_CODEC_ID_NONE)
                {
                    cid = av_codec_get_id(oformat->codec_tag, i++);
                    if(codecs.contains(cid))
                        qDebug() << "     " << codecs[cid];
                    else
                        qDebug() << "     " << "unknown codec";
                }
            }
            oformat = av_oformat_next(oformat);
        }
    }

    QStringList list;
    list.append("MPEG-4 (mp4)");
    list.append("Matroska (mkv)");
    list.append("Flash Video (flv)");
    list.append("QuickTime (mov)");

    return list;
}

VideoRecorder::VideoRecorder(QString output_filename, QString quality_name, QString format) :
    fmt(NULL),
    oc(NULL),
    video_st({0}),
    video_codec(NULL),
    opt(NULL),
    error(false),
    w(0),
    h(0),
    framecount(0),
    filename(output_filename),
    done(true),
    quality(quality_name),
    frame_time_s(0)
{
    av_register_all();

    if(filename.isEmpty()) {
        QString dirpath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        if(!QDir(dirpath + "/Vidiot").exists()) {
            if(QDir(dirpath).mkpath(dirpath + "/Vidiot"))
                dirpath += "/Vidiot";
        }
        else
            dirpath += "/Vidiot";

        filename = dirpath + "/Recording_" + QDateTime::currentDateTime().toString("yyyy_MM_dd-hh_mm_ss") + "." + format;
        filename.replace("//", "/");
    }

    char fname[1024];
    strcpy(fname, filename.toLocal8Bit().data());

    avformat_alloc_output_context2(&oc, NULL, NULL, fname);
    if(!oc) {
        qDebug() << "Couldn't determine video format automatically, using mp4";
        avformat_alloc_output_context2(&oc, NULL, "mp4", fname);
    }

    if(!oc)
        SET_ERROR_RETURN("No suitable output format found");

    fmt = oc->oformat;

    QSettings settings(QApplication::applicationDirPath() + "/quality.ini", QSettings::IniFormat);

    settings.beginGroup(quality_name);


//    if(!video_codec && codec_nvenc_h264) {
//        video_codec = avcodec_find_encoder(codec_nvenc_h264);
//        qDebug() << "Using nvenc";
//    }

//    if(!video_codec && codec_h264_qsv) {
//        video_codec = avcodec_find_encoder(codec_h264_qsv);
//        qDebug() << "Using qsv";
//    }

    if(!video_codec && codec_libx264) {
        video_codec = avcodec_find_encoder(codec_libx264);
        qDebug() << "Using x264";

        if(video_codec) {
            av_dict_set(&opt, "vprofile", settings.value("profile", "baseline").toByteArray().data(), 0);
            av_dict_set(&opt, "preset", settings.value("preset", "ultrafast").toByteArray().data(), 0);
            if(settings.contains("tune"))
                av_dict_set(&opt, "tune", settings.value("tune").toByteArray().data(), 0);
            if(settings.contains("crf") && !settings.contains("bitrate"))
                av_dict_set_int(&opt, "crf", settings.value("crf").toInt(), 0);
        }
    }

    if(!video_codec)
        video_codec = avcodec_find_encoder(fmt->video_codec);

    if(!video_codec)
        SET_ERROR_RETURN("Couldn't find video codec:" << avcodec_get_name(fmt->video_codec));

    video_st.st = avformat_new_stream(oc, NULL);
    if(!video_st.st)
        SET_ERROR_RETURN("Couldn't allocate video stream");

    video_st.st->id = oc->nb_streams-1;
    AVCodecContext *c = avcodec_alloc_context3(video_codec);
    if(!c)
        SET_ERROR_RETURN("Couldn't allocate encoding context");

    video_st.enc = c;

    c->codec_id = video_codec->id;
//    if(settings.contains("bitrate"))
//        c->bit_rate = 1000*settings.value("bitrate").toInt();

    /* width & height will be known later */

    video_st.st->time_base.num = 1;
    video_st.st->time_base.den = 1000;
    c->time_base = video_st.st->time_base;

    c->gop_size = settings.value("gop_size", 250).toInt();
    c->pix_fmt = AV_PIX_FMT_YUV420P;

    /* Some formats want stream headers to be separate. */
//    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
//        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    std::cout << "recording:" << filename.toLocal8Bit().data() << std::endl;

    done = false;
}

VideoRecorder::~VideoRecorder()
{
}


AVFrame *VideoRecorder::alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;
    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;
    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0)
        SET_ERROR_RETURN_NULL("Couldn't allocate frame data");

    return picture;
}

void VideoRecorder::initialize(int width, int height)
{
    if(w > 0)
        return;

    w = width + (width%2);
    h = height + (height%2);

    video_st.enc->width = w;
    video_st.enc->height = h;


    AVCodecContext *c = video_st.enc;
    int ret;
    AVDictionary *new_opt = NULL;

    av_dict_copy(&new_opt, opt, 0);
    ret = avcodec_open2(c, video_codec, &new_opt);
    av_dict_free(&new_opt);
    if(ret < 0) {
        char str[256]; av_make_error_string(str, 256, ret);
        SET_ERROR_RETURN("Couldn't open video codec:" << str);
    }

    video_st.frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if(!video_st.frame)
        SET_ERROR_RETURN("Coudln't allocate video frame");


    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    video_st.tmp_frame = NULL;
    if (c->pix_fmt != AV_PIX_FMT_YUV420P) {
        video_st.tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);
        if (!video_st.tmp_frame)
            SET_ERROR_RETURN("Could not allocate temporary picture");
    }
    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(video_st.st->codecpar, c);
    if (ret < 0)
        SET_ERROR_RETURN("Could not copy the stream parameters");





    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
        char fname[256];
        strcpy(fname, filename.toLocal8Bit().data());
        ret = avio_open(&oc->pb, fname, AVIO_FLAG_WRITE);
        if (ret < 0) {
            char str[256]; av_make_error_string(str, 256, ret);
            SET_ERROR_RETURN("Could not open output file:" << str);
        }
    }

    /* Write the stream header, if any. */
    ret = avformat_write_header(oc, &opt);
    if (ret < 0) {
        char str[256]; av_make_error_string(str, 256, ret);
        SET_ERROR_RETURN("Error occurred when opening output file:" << str);
    }

    av_dump_format(oc, 0, filename.toLocal8Bit().data(), 1);

    t.start();
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

void VideoRecorder::end()
{
    if(done)
        return;

    av_write_trailer(oc);

    close_stream(oc, &video_st);

    if (!(fmt->flags & AVFMT_NOFILE)) {
        /* Close the output file. */
        qDebug() << "Closed file";
        avio_closep(&oc->pb);
    }

    /* free the stream */
    avformat_free_context(oc);

    qDebug() << "Recording done";

    done = true;
}

void VideoRecorder::flushOne()
{
    if(done)
        return;

    /* no more frame to compress. The codec has a latency of a few
       frames if using B frames, so we get the last frames by
       passing the same picture again */

    if(!writeFrame(RecorderFrame()))
        end();
}

void VideoRecorder::flush()
{
    if(done)
        return;

    /* no more frame to compress. The codec has a latency of a few
       frames if using B frames, so we get the last frames by
       passing the same picture again */

    qDebug() << "Flushing frames";

    while (writeFrame(RecorderFrame())) {}

    qDebug() << "Flushing finished";

    end();
}

bool VideoRecorder::writeFrame(const RecorderFrame &frame)
{
    initialize(frame.width, frame.height);

    if(error)
        return false;


    int ret;
    int got_packet = 0;
    AVCodecContext *c = video_st.enc;

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    if(frame.data) {
        /* as we only generate a RGB24 picture, we must convert it
         * to the codec pixel format if needed */
        if (!video_st.sws_ctx) {
            video_st.sws_ctx = sws_getContext(frame.width, frame.height, AV_PIX_FMT_RGB24, c->width, c->height, c->pix_fmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);
            if (!video_st.sws_ctx)
                SET_ERROR_RETURN_FALSE("Could not initialize the conversion context");
        }
        const unsigned char *data_array[1] = {frame.data};
        const int stride_array[1] = {frame.width*3};
        sws_scale(video_st.sws_ctx, data_array, stride_array, 0, c->height, video_st.frame->data, video_st.frame->linesize);

        video_st.frame->pts = frame.time_ms;//video_st.next_pts++;

        frame_time_s = (float)video_st.frame->pts * (float)av_q2d(c->time_base);

        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, video_st.frame, &got_packet);
    }
    else {
        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, NULL, &got_packet);
    }

    if (ret < 0) {
        char str[256]; av_make_error_string(str, 256, ret);
        SET_ERROR_RETURN_FALSE("Error encoding video frame:" << str);
    }

    if (got_packet) {

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(&pkt, c->time_base, video_st.st->time_base);
        pkt.stream_index = video_st.st->index;
        /* Write the compressed frame to the media file. */
        //log_packet(fmt_ctx, pkt);
        //pkt.pts = pkt.dts = AV_NOPTS_VALUE;
        ret = av_interleaved_write_frame(oc, &pkt);

        if (ret < 0) {
            char str[256]; av_make_error_string(str, 256, ret);
            SET_ERROR_RETURN_FALSE("Error while writing video frame:" << str);
        }

        return true;
    }

    return false;
}

RecorderController::RecorderController() :
    QObject(NULL),
    recorder(NULL)
{
    QSettings settings;
    format = settings.value("format", "mp4").toString();
    quality = settings.value("quality", "Medium").toString();
    setObjectName("RecordController");
}

void RecorderController::process()
{
    if(frames.isEmpty())
        return;

    if(!recorder)
        recorder = new VideoRecorder(filename, quality, format);

    recorder->writeFrame(frames.first());
    if(frames.first().data)
        delete frames.first().data;
    frames.removeFirst();

    qDebug() << "Frames in queue" << frames.length();
}

void RecorderController::stop()
{
    if(recorder) {
        qDebug() << "Stopping recorder";

        while(frames.length()) {
            recorder->writeFrame(frames.first());
            if(frames.first().data)
                delete frames.first().data;
            frames.removeFirst();

            qDebug() << "Frames in queue" << frames.length();
        }

        recorder->flush();

        delete recorder;
        recorder = NULL;
    }

    while(frames.length()) {
        if(frames.first().data)
            delete frames.first().data;
        frames.removeFirst();

        qDebug() << "Frames in queue" << frames.length();
    }
}

void RecorderController::recorderFrame(RecorderFrame fr)
{

    long long bytes = frames.length()*fr.width*fr.height*3*sizeof(unsigned char);

    if(bytes < 1073741824LL) // 1 gb
        frames.append(fr);
    else {
        qDebug() << "Skipping frame!!!";
        delete fr.data;
    }

    qDebug() << "Record buffer at" << (int)(100*(float)bytes/1073741824.f) << "%";
}

void RecorderController::setFormat(QString format_str)
{
    if(format_str.contains("(")) {
        format = format_str.split("(").last();
        format.remove(")");
        qDebug() << "Format set to" << format;

        QSettings settings;
        settings.setValue("format", format_str);
    }
}

void RecorderController::setQuality(QString quality_str)
{
    quality = quality_str;
    qDebug() << "Quality set to" << quality;

    QSettings settings;
    settings.setValue("quality", quality);
}

