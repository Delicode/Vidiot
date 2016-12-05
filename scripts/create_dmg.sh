#!/bin/sh -e

cd "$(dirname "$0")"

CWD=`pwd`

VIDIOT_BUILD_CONFIG=../vidiot.build_config
source vidiot_build_details.sh
BUNDLE_PATH="${BUILD_DIR}/Vidiot.app"
CURRENT_DIR=$(pwd)

./create-dmg/create-dmg --window-size 589 381 --background ../resources/vidiot_bg.png --icon-size 96 --volname "Delicode Vidiöt v$VERSION" --icon "Applications" 310 158 --icon "Vidiot.app" 75 158 ./Vidiot_${VERSION}.dmg $BUNDLE_PATH

