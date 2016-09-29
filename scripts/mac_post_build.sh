#!/bin/sh -e

cd "$(dirname "$0")"

CWD=`pwd`

VIDIOT_BUILD_CONFIG=../vidiot.build_config
source vidiot_build_details.sh

BUNDLE_PATH="${BUILD_DIR}/Vidiot.app"

CURRENT_DIR=$(pwd)
QT_DIR=/Users/Delicode1/Qt56/5.6/clang_64/lib
QT_LIB_PATH=@rpath
QT_FOLDER=framework/Versions/5

if test -d $BUNDLE_PATH
then
    echo "Using ${BUNDLE_PATH}."
else
    echo "Application bundle not found: ${BUNDLE_PATH}!"
    exit 1
fi

if [ "$BUILD_MODE" == "DEBUG" ]; then
    echo "Copying QT frameworks (debug)..."
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

cp replace_rpaths.sh $BUNDLE_PATH/Contents/Frameworks
cp replace_debug_paths.sh $BUNDLE_PATH/Contents/Frameworks
cp replace_plugin_rpaths.sh $BUNDLE_PATH/Contents/PlugIns/imageformats
cp replace_plugin_debug_paths.sh $BUNDLE_PATH/Contents/PlugIns/imageformats
cp replace_plugin_rpaths.sh $BUNDLE_PATH/Contents/PlugIns/printsupport
cp replace_plugin_debug_paths.sh $BUNDLE_PATH/Contents/PlugIns/printsupport
cp replace_plugin_rpaths.sh $BUNDLE_PATH/Contents/PlugIns/bearer
cp replace_plugin_debug_paths.sh $BUNDLE_PATH/Contents/PlugIns/bearer

cd $BUNDLE_PATH/Contents/Frameworks

sh replace_rpaths.sh
cd $BUNDLE_PATH/Contents/plugins/imageformats
sh replace_plugin_rpaths.sh
cd $BUNDLE_PATH/Contents/plugins/printsupport
sh replace_plugin_rpaths.sh
cd $BUNDLE_PATH/Contents/plugins/bearer
sh replace_plugin_rpaths.sh

if [ "$BUILD_MODE" = "DEBUG" ]; then
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

echo "Copying ffmpeg libs"
cp -v /usr/local/opt/ffmpeg/lib/libavdevice.57.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v /usr/local/opt/ffmpeg/lib/libavcodec.57.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v /usr/local/opt/ffmpeg/lib/libavformat.57.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v /usr/local/opt/ffmpeg/lib/libswscale.4.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v /usr/local/opt/ffmpeg/lib/libavutil.55.dylib $BUNDLE_PATH/Contents/Frameworks/
cp -v /usr/local/opt/ffmpeg/lib/libswresample.2.dylib $BUNDLE_PATH/Contents/Frameworks/

echo "copying syphon"
cp -rv ../lib/Syphon.framework $BUNDLE_PATH/Contents/Frameworks/

echo "changing ffmpeg search paths"

install_name_tool -change /usr/local/opt/ffmpeg/lib/libavdevice.57.dylib @executable_path/../Frameworks/libavdevice.57.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change /usr/local/opt/ffmpeg/lib/libavcodec.57.dylib @executable_path/../Frameworks/libavcodec.57.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change /usr/local/opt/ffmpeg/lib/libavformat.57.dylib @executable_path/../Frameworks/libavformat.57.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change /usr/local/opt/ffmpeg/lib/libswscale.4.dylib @executable_path/../Frameworks/libswscale.4.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change /usr/local/opt/ffmpeg/lib/libavutil.55.dylib @executable_path/../Frameworks/libavutil.55.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change /usr/local/opt/ffmpeg/lib/libswresample.2.dylib @executable_path/../Frameworks/libswresample.2.dylib $BUNDLE_PATH/Contents/MacOS/Vidiot

echo "changing QtAv search paths"
install_name_tool -change @rpath/QtAV.framework/Versions/1/QtAV @executable_path/../Frameworks/QtAV.framework/Versions/1/QtAV $BUNDLE_PATH/Contents/MacOS/Vidiot
install_name_tool -change @rpath/QtAVWidgets.framework/Versions/1/QtAVWidgets @executable_path/../Frameworks/QtAvWidgets.framework/Versions/1/QtAvWidgets $BUNDLE_PATH/Contents/MacOS/Vidiot

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
fi
