
QT       += core gui opengl widgets quick concurrent

TARGET = Vidiot
TEMPLATE = app


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
		StdInThread.h

INCLUDEPATH += lib/ffmpeg/include

LIBS += -lavdevice -lavcodec -lavformat -lswscale -lavutil -lswresample

INCLUDEPATH += "../Vidiot/lib/QtAV/include/QtAV" "../Vidiot/lib/QtAV/include"

CONFIG(release, debug|release) {
	LIBS += -lQtAV1
}
else {
	LIBS += -lQtAVd1
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

macx {
	QMAKE_INFO_PLIST = resources/MyInfo.plist
	LIBS += -framework Syphon
}

DISTFILES += \
	qml/main.qml \
	qml/SimpleButton.qml \
	qml/SimpleList.qml

RESOURCES += qml.qrc \
    media.qrc
