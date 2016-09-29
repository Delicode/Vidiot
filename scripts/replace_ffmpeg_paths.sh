#!/bin/sh -e

FFMPEG_FOLDER=/usr/local/opt/ffmpeg/lib
AVDEVICE_VERSION=57
AVCODEC_VERSION=57
AVFORMAT_VERSION=57
SWSCALE_VERSION=4
AVUTIL_VERSION=55
SWRESAMPLE_VERSION=2
AVRESAMPLE_VERSION=3
AVFILTER_VERSION=6
X264_VERSION=148
MP3_VERSION=0
POSTPROC_VERSION=54

FFMPEG_LIBS=(
    libavfilter.$AVFILTER_VERSION.dylib
    libswscale.$SWSCALE_VERSION.dylib
    libavformat.$AVFORMAT_VERSION.dylib
    libx264.$X264_VERSION.dylib
    libavresample.$AVRESAMPLE_VERSION.dylib
    libavutil.$AVUTIL_VERSION.dylib
    libmp3lame.$MP3_VERSION.dylib
    libavcodec.$AVCODEC_VERSION.dylib
    libpostproc.$POSTPROC_VERSION.dylib
    libavdevice.$AVDEVICE_VERSION.dylib
    libswresample.$SWRESAMPLE_VERSION.dylib
)

X264_PATH=/usr/local/opt/x264/lib/libx264.$X264_VERSION.dylib
MP3_PATH=/usr/local/opt/lame/lib/libmp3lame.$MP3_VERSION.dylib

for FILE in ${FFMPEG_LIBS[@]}; do
    echo "Handling file  " $FILE

    # Get all dependencies under cellar
    LIBS=`otool -L $FILE | grep '/usr/local/Cellar/ffmpeg/3.1.3/lib' | awk '{ print $1 }'`
    for LIB in $LIBS
    do
        # change their paths to be found under frameworks
        EXEPATH=`echo $LIB | sed 's/\/usr\/local\/Cellar\/ffmpeg\/3.1.3\/lib\//@executable_path\/..\/Frameworks\//'`
        LIBNAME=`echo $LIB | sed 's/\/usr\/local\/Cellar\/ffmpeg\/3.1.3\/lib\///'`
        install_name_tool -change $LIB $EXEPATH $FILE
    done

    # change the id for the library itself
    install_name_tool -id @executable_path/../Frameworks/$FILE $FILE

    # If the file uses these libs, change them as well
    install_name_tool -change $X264_PATH @executable_path/../Frameworks/libx264.$X264_VERSION.dylib $FILE
    install_name_tool -change $MP3_PATH @executable_path/../Frameworks/libmp3lame.$MP3_VERSION.dylib $FILE
done

