QT += core gui opengl widgets quick concurrent

TARGET = Vidiot
TEMPLATE = app
MAJOR_VERSION = 1
MINOR_VERSION = 00
REVISION = $$system(git log --pretty=format:%h -n 1)
DEFINES += PRODUCT_REVISION=r$$REVISION
DEFINES += MAJOR_VERSION=$$MAJOR_VERSION
DEFINES += MINOR_VERSION=$$MINOR_VERSION
DEFINES += DELICODE_BUILD_TIMESTAMP=$$system(git log --pretty=format:%ct -n 1)


SOURCES += main.cpp\
		VideoView.cpp \
		TextureUploader.cpp \
		TextureDownloader.cpp \
		VideoPlayer.cpp \
		VideoThread.cpp \
		VideoRecorder.cpp \
		FeedInput.cpp \
		FboRenderer.cpp \
		FeedOutput.cpp \
		StdInThread.cpp

HEADERS  += VideoView.h \
		TextureUploader.h \
		TextureDownloader.h \
		VideoPlayer.h \
		VideoThread.h \
		VideoRecorder.h \
		FeedInput.h \
		FboRenderer.h \
		FeedOutput.h \
		StdInThread.h \
    predefines.h

INCLUDEPATH += lib/ffmpeg/include

macx {
    LIBS += -L/usr/local/lib
}
win32 {
    LIBS += -L$$_PRO_FILE_PWD_/lib/ffmpeg/lib/win64
}
LIBS += -lavdevice -lavcodec -lavformat -lswscale -lavutil -lswresample

INCLUDEPATH += "../Vidiot/lib/QtAV/include/QtAV" "../Vidiot/lib/QtAV/include"

win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$$_PRO_FILE_PWD_/lib/QtAV/lib/win64 -lQtAV1
    }
    else {
        LIBS += -L$$_PRO_FILE_PWD_/lib/QtAV/lib/win64 -lQtAVd1
    }
}

win32: {
	SPOUTDIR = external/Spout
	INCLUDEPATH += $${SPOUTDIR}
	LIBS += -lShell32 -lUser32 -lOpenGL32 -lGdi32
	LIBS += -lStrmiids -lQuartz -lole32 -luuid -loleaut32

	RC_FILE = resources/info.rc

	!contains(QMAKE_TARGET.arch, x86_64) {
		LIBS += -L"../Vidiot/lib/QtAV/lib/win32" -L"../Vidiot/lib/ffmpeg/win32"

	} else {
		LIBS += -L"../Vidiot/lib/QtAV/lib/win64" -L"../Vidiot/lib/ffmpeg/win64"

	}

	SOURCES += $${SPOUTDIR}/SpoutDirectX.cpp \
				$${SPOUTDIR}/SpoutGLDXinterop.cpp \
				$${SPOUTDIR}/SpoutGLextensions.cpp \
				$${SPOUTDIR}/SpoutMemoryShare.cpp \
				$${SPOUTDIR}/SpoutReceiver.cpp \
				$${SPOUTDIR}/SpoutSDK.cpp \
				$${SPOUTDIR}/SpoutSender.cpp \
				$${SPOUTDIR}/SpoutSenderNames.cpp \
				$${SPOUTDIR}/SpoutSharedMemory.cpp

	HEADERS += $${SPOUTDIR}/Spout.h \
				$${SPOUTDIR}/SpoutCommon.h \
				$${SPOUTDIR}/SpoutDirectX.h \
				$${SPOUTDIR}/SpoutGLDXinterop.h \
				$${SPOUTDIR}/SpoutGLextensions.h \
				$${SPOUTDIR}/SpoutMemoryShare.h \
				$${SPOUTDIR}/SpoutReceiver.h \
				$${SPOUTDIR}/SpoutSDK.h \
				$${SPOUTDIR}/SpoutSender.h \
				$${SPOUTDIR}/SpoutSenderNames.h \
				$${SPOUTDIR}/SpoutSharedMemory.h
}

defineReplace(firstPBC){
    return("$$1>\"$$_PRO_FILE_PWD_/vidiot.build_config\"$$escape_expand(\\n\\t)")
}
defineReplace(addPBC){
    return("$$1>>\"$$_PRO_FILE_PWD_/vidiot.build_config\"$$escape_expand(\\n\\t)")
}
defineReplace(addPostLink){
    return("$$1$$escape_expand(\\n\\t)")
}

#create vidiot.build_config file to notify installer scripts about the current build
CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$firstPBC(echo $$TARGET)
    QMAKE_POST_LINK += $$addPBC(echo RELEASE)
    QMAKE_POST_LINK += $$addPBC(echo $${MAJOR_VERSION}.$${MINOR_VERSION})
    QMAKE_POST_LINK += $$addPBC(echo "$$REVISION")
    QMAKE_POST_LINK += $$addPBC(echo $$_PRO_FILE_PWD_/Resources)
    QMAKE_POST_LINK += $$addPBC(echo $$OUT_PWD/$$DESTDIR)
    QMAKE_POST_LINK += $$addPBC(echo "$$BRANCH")
}


#create products.build_config file to notify installer scripts about the current build
CONFIG(debug, debug|release) {
    QMAKE_POST_LINK += $$firstPBC(echo $$TARGET)
    QMAKE_POST_LINK += $$addPBC(echo DEBUG)
    QMAKE_POST_LINK += $$addPBC(echo $${MAJOR_VERSION}.$${MINOR_VERSION})
    QMAKE_POST_LINK += $$addPBC(echo "$$REVISION")
    QMAKE_POST_LINK += $$addPBC(echo $$_PRO_FILE_PWD_/Resources)
    QMAKE_POST_LINK += $$addPBC(echo $$OUT_PWD/$$DESTDIR)
    QMAKE_POST_LINK += $$addPBC(echo "$$BRANCH")
}


macx {
    OBJECTIVE_SOURCES += DelicodeSyphonServer.mm
    HEADERS += DelicodeSyphonServer.h
	QMAKE_INFO_PLIST = resources/MyInfo.plist
    LIBS += -framework IOKit -framework CoreFoundation -framework CoreServices -framework ApplicationServices -framework AppKit

    QMAKE_LFLAGS += -F$$_PRO_FILE_PWD_/lib/QtAV/lib/osx
    LIBS += -framework QtAV -framework QtAVWidgets

    QMAKE_LFLAGS += -F$$_PRO_FILE_PWD_/lib
    LIBS += -framework Syphon

    QMAKE_POST_LINK += $$addPostLink($$_PRO_FILE_PWD_/scripts/mac_post_build.sh)
}

DISTFILES += \
	qml/main.qml \
	qml/SimpleButton.qml \
	qml/SimpleList.qml

RESOURCES += qml.qrc \
    media.qrc
