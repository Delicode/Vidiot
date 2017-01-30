#!/bin/sh -e
#
# This script creates an app bundle out of the compiled binary
# The script copies assorted lib files into the bundle and then changes all @rpath and @loader_path
# into @executable_path as we've found that this method gives us the least headache later.
# This script does not build the .dmg

cd "$(dirname "$0")"

CWD=`pwd`

VIDIOT_BUILD_CONFIG=../vidiot.build_config
source vidiot_build_details.sh

BUNDLE_PATH="${BUILD_DIR}/Vidiot.app"

CURRENT_DIR=$(pwd)

# TODO: make these dynamic
QT_DIR=/Users/Delicode1/Qt56/5.6/clang_64/lib
QT_QML_DIR=/Users/Delicode1/Qt56/5.6/clang_64/qml
QT_LIB_PATH=@rpath
QT_FOLDER=framework/Versions/5
FFMPEG_FOLDER=/usr/local/opt/ffmpeg/lib
FFMPEG_VERSION=3.2.2
AVDEVICE_VERSION=57
AVCODEC_VERSION=57
AVFORMAT_VERSION=57
SWSCALE_VERSION=4
AVUTIL_VERSION=55
SWRESAMPLE_VERSION=2
AVRESAMPLE_VERSION=3
AVFILTER_VERSION=6
ICONV_VERSION=2
QT_VERSION=5
QTAV_VERSION=1

if [ "$BUILD_MODE" == "DEBUG" ]; then
    # You must rename a couple of *_debug.dylib libs into *.dylib
    # in the QtAV build dir as well as the QML build dir
    # The build dir is usually QtAV_build/lib_osx_x86_64_llvm
    # and QML dir QtAV_build/bin
    QTAV_QML_DIR=/Users/Delicode1/QtAV_build_debug/bin/
else
    QTAV_QML_DIR=/Users/Delicode1/QtAV_build/bin/
fi

if test -d $BUNDLE_PATH
then
    echo "Using ${BUNDLE_PATH}."
else
    echo "Application bundle not found: ${BUNDLE_PATH}!"
    exit 1
fi

if [ "$BUILD_MODE" == "DEBUG" ]; then
    echo "Copying QT frameworks (debug)..."
    # When calling macdeployqt it's necessary to specify -use-debug-libs if you want to step through
    # the code in the app bundle. Not using the flag appears to strip any debuggign information
    ~/Qt56/5.6/clang_64/bin/macdeployqt $BUNDLE_PATH -use-debug-libs
fi

if [ "$BUILD_MODE" == "RELEASE" ]; then
    echo "Copying QT frameworks..."
    ~/Qt56/5.6/clang_64/bin/macdeployqt $BUNDLE_PATH
fi

echo "copying Qt Av"
cp -rv ../lib/QtAV/lib/osx/QtAV.framework $BUNDLE_PATH/Contents/Frameworks/
cp -rv ../lib/QtAV/lib/osx/QtAVWidgets.framework $BUNDLE_PATH/Contents/Frameworks/
cp -rv ../lib/QtAV/lib/osx/libQmlAV.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -rv ../lib/QtAV/lib/osx/libcommon.a $BUNDLE_PATH/Contents/Frameworks/
cp -v ../LICENSE $BUNDLE_PATH/Contents/Resources

# These scripts walk through all libraries and replace assorted @rpath references between them
# into hardcoded @executable_path references
cp replace_rpaths.sh $BUNDLE_PATH/Contents/Frameworks
cp replace_debug_paths.sh $BUNDLE_PATH/Contents/Frameworks
cp replace_plugin_rpaths.sh $BUNDLE_PATH/Contents/PlugIns/imageformats
cp replace_plugin_debug_paths.sh $BUNDLE_PATH/Contents/PlugIns/imageformats
cp replace_plugin_rpaths.sh $BUNDLE_PATH/Contents/PlugIns/printsupport
cp replace_plugin_debug_paths.sh $BUNDLE_PATH/Contents/PlugIns/printsupport
cp replace_plugin_rpaths.sh $BUNDLE_PATH/Contents/PlugIns/bearer
cp replace_plugin_debug_paths.sh $BUNDLE_PATH/Contents/PlugIns/bearer

chmod -R 755 $BUNDLE_PATH/Contents/PlugIns
chmod -R 755 $BUNDLE_PATH/Contents/Frameworks

cd $BUNDLE_PATH/Contents/Frameworks

sh replace_rpaths.sh
cd $BUNDLE_PATH/Contents/plugins/imageformats
sh replace_plugin_rpaths.sh
cd $BUNDLE_PATH/Contents/plugins/printsupport
sh replace_plugin_rpaths.sh
cd $BUNDLE_PATH/Contents/plugins/bearer
sh replace_plugin_rpaths.sh

if [ "$BUILD_MODE" = "DEBUG" ]; then

    # If macdeployqt is called with the debug flag the Qt libs are trying to search for
    # *_debug versions of the other Qt libs inside the app bundle. So just change them all to
    # search for non-debug filenames, even though the libs actually are debug versions.
    cd $BUNDLE_PATH/Contents/Frameworks
    echo "Replacing debug paths"
    sh replace_debug_paths.sh
    cd $BUNDLE_PATH/Contents/plugins/imageformats
    sh replace_plugin_debug_paths.sh
    cd $BUNDLE_PATH/Contents/plugins/printsupport
    sh replace_plugin_debug_paths.sh
    cd $BUNDLE_PATH/Contents/plugins/bearer
    sh replace_plugin_debug_paths.sh
fi

cd $CWD

install_name_tool -id @executable_path/../Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change @rpath/QtGui.framework/Versions/$QT_VERSION/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/$QT_VERSION/QtGui $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change @rpath/QtCore.framework/Versions/$QT_VERSION/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/$QT_VERSION/QtCore $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV

install_name_tool -id @executable_path/../Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change @rpath/QtAV.framework/Versions/$QTAV_VERSION/QtAV @executable_path/../Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change @rpath/QtGui.$QT_FOLDER/QtGui @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change @rpath/QtCore.$QT_FOLDER/QtCore @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change @rpath/QtOpenGL.$QT_FOLDER/QtOpenGL @executable_path/../Frameworks/QtOpenGL.$QT_FOLDER/QtOpenGL $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change @rpath/QtWidgets.$QT_FOLDER/QtWidgets @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets

echo "changing QtAv search paths"
install_name_tool -change @rpath/QtAV.framework/Versions/$QTAV_VERSION/QtAV @executable_path/../Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets @executable_path/../Frameworks/QtAvWidgets.framework/Versions/$QTAV_VERSION/QtAvWidgets $BUNDLE_PATH/Contents/MacOS/Vidiot

echo "changing syphon search paths"
install_name_tool -change @loader_path/../Frameworks/Syphon.framework/Versions/A/Syphon @executable_path/../Frameworks/Syphon.framework/Versions/A/Syphon $BUNDLE_PATH/Contents/MacOS/Vidiot

echo "changing Qt search paths"
install_name_tool -change @rpath/QtCore.$QT_FOLDER/QtCore @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtOpenGL.$QT_FOLDER/QtOpenGL @executable_path/../Frameworks/QtOpenGL.$QT_FOLDER/QtOpenGL $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtWidgets.$QT_FOLDER/QtWidgets @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtGui.$QT_FOLDER/QtGui @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtQuick.$QT_FOLDER/QtQuick @executable_path/../Frameworks/QtQuick.$QT_FOLDER/QtQuick $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtQml.$QT_FOLDER/QtQml @executable_path/../Frameworks/QtQml.$QT_FOLDER/QtQml $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtNetwork.$QT_FOLDER/QtNetwork @executable_path/../Frameworks/QtNetwork.$QT_FOLDER/QtNetwork $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtConcurrent.$QT_FOLDER/QtConcurrent @executable_path/../Frameworks/QtConcurrent.$QT_FOLDER/QtConcurrent $BUNDLE_PATH/Contents/MacOS/Vidiot

install_name_tool -change @rpath/QtCore.$QT_FOLDER/QtCore @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtOpenGL.$QT_FOLDER/QtOpenGL @executable_path/../Frameworks/QtOpenGL.$QT_FOLDER/QtOpenGL $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtWidgets.$QT_FOLDER/QtWidgets @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtGui.$QT_FOLDER/QtGui @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtQuick.$QT_FOLDER/QtQuick @executable_path/../Frameworks/QtQuick.$QT_FOLDER/QtQuick $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtQml.$QT_FOLDER/QtQml @executable_path/../Frameworks/QtQml.$QT_FOLDER/QtQml $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtNetwork.$QT_FOLDER/QtNetwork @executable_path/../Frameworks/QtNetwork.$QT_FOLDER/QtNetwork $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtConcurrent.$QT_FOLDER/QtConcurrent @executable_path/../Frameworks/QtConcurrent.$QT_FOLDER/QtConcurrent $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtDBus.$QT_FOLDER/QtDBus @executable_path/../Frameworks/QtDBus.$QT_FOLDER/QtDBus $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change @rpath/QtPrintSupport.$QT_FOLDER/QtPrintSupport @executable_path/../Frameworks/QtPrintSupport.$QT_FOLDER/QtPrintSupport $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib

if [ "$BUILD_MODE" = "DEBUG" ]; then
    echo "fixing debug dylib paths"
    install_name_tool -change @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtOpenGL.$QT_FOLDER/QtOpenGL @executable_path/../Frameworks/QtOpenGL.$QT_FOLDER/QtOpenGL_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtQuick.$QT_FOLDER/QtQuick @executable_path/../Frameworks/QtQuick.$QT_FOLDER/QtQuick_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtQml.$QT_FOLDER/QtQml @executable_path/../Frameworks/QtQml.$QT_FOLDER/QtQml_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtNetwork.$QT_FOLDER/QtNetwork @executable_path/../Frameworks/QtNetwork.$QT_FOLDER/QtNetwork_debug $BUNDLE_PATH/Contents/MacOS/Vidiot
    install_name_tool -change @executable_path/../Frameworks/QtConcurrent.$QT_FOLDER/QtConcurrent @executable_path/../Frameworks/QtConcurrent.$QT_FOLDER/QtConcurrent_debug $BUNDLE_PATH/Contents/MacOS/Vidiot

    install_name_tool -id @executable_path/../PlugIns/platforms/libqcocoa.dylib $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
    install_name_tool -change @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore @executable_path/../Frameworks/QtCore.$QT_FOLDER/QtCore_debug $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
    install_name_tool -change @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui @executable_path/../Frameworks/QtGui.$QT_FOLDER/QtGui_debug $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
    install_name_tool -change @executable_path/../Frameworks/QtDBus.$QT_FOLDER/QtDBus @executable_path/../Frameworks/QtDBus.$QT_FOLDER/QtDBus_debug $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
    install_name_tool -change @executable_path/../Frameworks/QtPrintSupport.$QT_FOLDER/QtPrintSupport @executable_path/../Frameworks/QtPrintSupport.$QT_FOLDER/QtPrintSupport_debug $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib
    install_name_tool -change @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets @executable_path/../Frameworks/QtWidgets.$QT_FOLDER/QtWidgets_debug $BUNDLE_PATH/Contents/PlugIns/platforms/libqcocoa.dylib

    install_name_tool -change @executable_path/../Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV_debug @executable_path/../Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
fi

echo "copying QML modules"
if [ ! -d $BUNDLE_PATH/Contents/Resources/qml ]; then
    mkdir $BUNDLE_PATH/Contents/Resources/qml
fi

cp -vr $QT_QML_DIR/Qt $BUNDLE_PATH/Contents/Resources/qml
cp -vr $QT_QML_DIR/QtQuick $BUNDLE_PATH/Contents/Resources/qml
cp -vr $QT_QML_DIR/QtQuick.2 $BUNDLE_PATH/Contents/Resources/qml
cp -vr $QTAV_QML_DIR/QtAV $BUNDLE_PATH/Contents/Resources/qml

cp replace_qml_rpaths.sh $BUNDLE_PATH/Contents/Resources/qml
chmod -R 755 $BUNDLE_PATH/Contents/Resources/qml
cd $BUNDLE_PATH/Contents/Resources/qml

sh replace_qml_rpaths.sh

cd $CWD

if [ "$BUILD_MODE" = "DEBUG" ]; then
    cp replace_qml_debug_paths.sh $BUNDLE_PATH/Contents/Resources/qml
    cd $BUNDLE_PATH/Contents/Resources/qml
    sh replace_qml_debug_paths.sh
    install_name_tool -change @executable_path/../Frameworks/QtAV.framework/Versions/1/QtAV_debug @executable_path/../Frameworks/QtAV.framework/Versions/1/QtAV $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
fi

cd $CWD

echo "Copying ffmpeg libs"
cp -v $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v /usr/lib/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks

cp -v /usr/local/opt/x264/lib/libx264.148.dylib $BUNDLE_PATH/Contents/Frameworks
cp -v /usr/local/opt/lame/lib/libmp3lame.0.dylib $BUNDLE_PATH/Contents/Frameworks

echo "copying syphon"
cp -rv ../lib/Syphon.framework $BUNDLE_PATH/Contents/Frameworks/

chmod -R 755 $BUNDLE_PATH/Contents/Frameworks

echo "changing ffmpeg search paths"

install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change $FFMPEG_FOLDER/lib/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot

install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV

install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
install_name_tool -change $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV

install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets

install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
install_name_tool -change $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets

install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib

install_name_tool -change @rpath/QtGui$QT_FOLDER/QtGui i@executable_path/../Frameworks/QtGui$QT_FOLDER/QtGui $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change @rpath/QtCore$QT_FOLDER/QtCore i@executable_path/../Frameworks/QtCore$QT_FOLDER/QtCore $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change @rpath/QtQuick$QT_FOLDER/QtQuick i@executable_path/../Frameworks/QtQuick$QT_FOLDER/QtQuick $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change @rpath/QtQml$QT_FOLDER/QtQml i@executable_path/../Frameworks/QtQml$QT_FOLDER/QtQml $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib
install_name_tool -change @rpath/QtNetwork$QT_FOLDER/QtNetwork i@executable_path/../Frameworks/QtNetwork$QT_FOLDER/QtNetwork $BUNDLE_PATH/Contents/Resources/qml/QtAV/libQmlAV.dylib

install_name_tool -change $FFMPEG_FOLDER/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavdevice.$AVDEVICE_VERSION.dylib @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib
install_name_tool -change $FFMPEG_FOLDER/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libQmlAV.dylib

if [ "$BUILD_MODE" = "DEBUG" ]; then
    echo "changing ffmpeg search paths (debug)"

    install_name_tool -change @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib_debug @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib_debug @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib_debug @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib_debug @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib_debug @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib_debug @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/QtAV

    install_name_tool -change @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib_debug @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib_debug @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib_debug @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib_debug @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib_debug @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV
    install_name_tool -change @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib_debug @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAV.framework/Versions/$QTAV_VERSION/QtAV

    install_name_tool -change @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib_debug @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib_debug @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib_debug @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib_debug @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib_debug @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib_debug @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/QtAVWidgets

    install_name_tool -change @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib_debug @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib_debug @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib_debug @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib_debug @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib_debug @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib_debug @executable_path/../Frameworks/libavdevice.$AVDEVICE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
    install_name_tool -change @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib_debug @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/QtAVWidgets.framework/Versions/$QTAV_VERSION/QtAVWidgets
fi


install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavcodec.$AVCODEC_VERSION.dylib
install_name_tool -change @rpath/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavcodec.$AVCODEC_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavcodec.$AVCODEC_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change @rpath/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change @rpath/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change @rpath/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change @rpath/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change @rpath/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change @rpath/libavfilter.$AVFILTER_VERSION.dylib @executable_path/../Frameworks/libavfilter.$AVFILTER_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavdevice.$AVDEVICE_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib
install_name_tool -change @rpath/libswscale.$SWSCALE_VERSION.dylib @executable_path/../Frameworks/libswscale.$SWSCALE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib
install_name_tool -change @rpath/libavresample.$AVRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib
install_name_tool -change @rpath/libavformat.$AVFORMAT_VERSION.dylib @executable_path/../Frameworks/libavformat.$AVFORMAT_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib
install_name_tool -change @rpath/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib
install_name_tool -change @rpath/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavfilter.$AVFILTER_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavformat.$AVFORMAT_VERSION.dylib
install_name_tool -change @rpath/libavcodec.$AVCODEC_VERSION.dylib @executable_path/../Frameworks/libavcodec.$AVCODEC_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavformat.$AVFORMAT_VERSION.dylib
install_name_tool -change @rpath/libswresample.$SWRESAMPLE_VERSION.dylib @executable_path/../Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavformat.$AVFORMAT_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavformat.$AVFORMAT_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavresample.$AVRESAMPLE_VERSION.dylib

install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libavutil.$AVUTIL_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libswresample.$SWRESAMPLE_VERSION.dylib

install_name_tool -change @rpath/libavutil.$AVUTIL_VERSION.dylib @executable_path/../Frameworks/libavutil.$AVUTIL_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libswscale.$SWSCALE_VERSION.dylib
install_name_tool -change /usr/lib/libiconv.$ICONV_VERSION.dylib @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libswscale.$SWSCALE_VERSION.dylib

install_name_tool -id @executable_path/../Frameworks/libiconv.$ICONV_VERSION.dylib $BUNDLE_PATH/Contents/Frameworks/libiconv.$ICONV_VERSION.dylib

cd $CWD
cp replace_ffmpeg_paths.sh $BUNDLE_PATH/Contents/Frameworks/
cd $BUNDLE_PATH/Contents/Frameworks
sh replace_ffmpeg_paths.sh

cd $CWD

# The quality.ini is crucial as Vidiöt won't run without it
cp ../quality.ini $BUNDLE_PATH/Contents/MacOS

# The .plist file is necessary to make the app into a background app by default
# This prevents the icon from popping up annoyingly when Vidiöt is executed in background mode
cp ../resources/MyInfo.plist $BUNDLE_PATH/Contents/Info.plist
cp ../resources/Vidiot.icns $BUNDLE_PATH/Contents/Resources/icon.icns

