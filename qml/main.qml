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

import QtQuick 2.4
import videoview 1.0
import "ionicons.js" as Ionicons
import QtQuick.Dialogs 1.2

import QtAV 1.6


Item {
    id: main
    anchors.fill: parent

    property real margin: 10*dpi
    property real radius: 5*dpi

    function timestring(seconds) {
        seconds -= 0.5;
        if(seconds < 0) seconds = 0;
        var m = Math.floor(seconds/60);
        var s = seconds-m*60;
        if(s.toFixed(0).length < 2)
            return m + ":0" + s.toFixed(0);
        else
            return m + ":" + s.toFixed(0);
    }

    function timestring_ms(mseconds) {
        return timestring(mseconds*0.001);
    }

    function setTarget(targetText) {
        if(targetText.indexOf("file:///") == 0) {
            targetText.replace("file:///", "");
            player.stop();
            player.source = targetText;
            player.play();
        }
        else {
            player.stop();
        }

        video.setSource(targetText);
        camera_settings.enabled = targetText.indexOf("dshow:///") == 0;
    }

    FontLoader {
        id: ionicons
        source: "ionicons.ttf"
        onStatusChanged: if (ionicons.status == FontLoader.Ready) console.log('Loaded')
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            sourcebutton.checked = false;
            resolutionbutton.checked = false;
            formatbutton.checked = false;
            qualitybutton.checked = false;
            sidebuttons.checked = false;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Image {
        source: "../resources/vidiot_bg.png"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 2*main.margin
        fillMode: Image.PreserveAspectFit
    }

    VideoView {
        id: video
        anchors.fill: parent

        opacity: sources.currentItem.text == "None" ? 0 : 1
        Behavior on opacity{NumberAnimation{duration: 100}}

        onDurationChanged: {
            if(duration > 0)
                playbutton.checked = true;
        }

        Text {
            visible: sources.currentItem.text != "None"
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: main.margin
            text: video.fps.toFixed(0) + " FPS";
            font.pixelSize: 20*0.8*dpi
            color: "white"
            style: Text.Outline
            styleColor: "black"
        }

        onError: {
            error_text.explanation = error_str;
            error_timer.stop();
            error_timer.start();
        }
    }

    Rectangle {
        id: menu_shade
        anchors.fill: parent
        color: "black"
        opacity: sources.currentItem.text == "None" || droparea.hover || sidebuttons.checked ? 0.5 : 0
        Behavior on opacity{NumberAnimation{duration: 100}}
    }

    Timer {
        id: error_timer
        interval: 5000
    }

    AVPlayer {
        id: player
        loops: AVPlayer.Infinite
        muted: true
    }

    Column {
        id: sidebuttons
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: main.margin
        spacing: main.margin
        width: parent.width-2*main.margin
        property bool checked: false

        SimpleButton {
            id: sourcebutton
            width: parent.width
            height: 20*dpi
            icon: checked ? "ion-arrow-left-c" : "ion-image"
            explanation: "Input - " + sources.humanreadable(sources.currentItem.text)
            onClicked: { checked = !checked; sidebuttons.checked = checked; video.updateSources();}
            disabled: sidebuttons.checked && !checked
        }

        SimpleButton {
            id: resolutionbutton
            width: parent.width
            height: 20*dpi
            icon: checked ? "ion-arrow-left-c" : "ion-qr-scanner"
            explanation: "Resolution - " + resolutions.currentItem.text
            onClicked: { checked = !checked; sidebuttons.checked = checked;}
            disabled: sidebuttons.checked && !checked
        }

        SimpleButton {
            id: formatbutton
            width: parent.width
            height: 20*dpi
            icon: checked ? "ion-arrow-left-c" : "ion-film-marker"
            explanation: "Format - " + formats.currentItem.text
            onClicked: { checked = !checked; sidebuttons.checked = checked;}
            disabled: sidebuttons.checked && !checked
        }

        SimpleButton {
            id: qualitybutton
            width: parent.width
            height: 20*dpi
            icon: checked ? "ion-arrow-left-c" : "ion-connection-bars"
            explanation: "Quality - " + qualities.currentItem.text
            onClicked: { checked = !checked; sidebuttons.checked = checked;}
            disabled: sidebuttons.checked && !checked
        }

        SimpleButton {
            id: recordbutton
            width: parent.width
            height: 20*dpi
            icon: checked ? "ion-stop" : "ion-record"
            explanation: checked ? "Stop recording" : "Record"
            onClicked: { checked = !checked; sidebuttons.checked = checked; video.record(checked);}
            disabled: (sidebuttons.checked && !checked) || sources.currentItem.text == "None" || sources.currentItem.text.indexOf("file:///") == 0
        }

    }

    SimpleList {
        id: sources
        anchors.left: sidebuttons.left
        anchors.leftMargin: main.margin + 20*dpi
        y: 20*dpi + 2*main.margin
        model: sourcelist
        open: sourcebutton.checked ? 1 : 0

        onTargetTextChanged: {
            sourcebutton.checked = false;
            sidebuttons.checked = false;

            if(targetText == "Open file...")
                filedialog.open();
            else {
                main.setTarget(targetText);
            }
        }
    }
    SimpleList {
        id: resolutions
        anchors.left: sidebuttons.left
        anchors.leftMargin: main.margin + 20*dpi
        y: 40*dpi + 3*main.margin
        model: resolutionlist
        open: resolutionbutton.checked ? 1 : 0
        onTargetTextChanged: {
            resolutionbutton.checked = false;
            sidebuttons.checked = false;
            video.setResolution(targetText);
        }
    }

    SimpleList {
        id: formats
        anchors.left: sidebuttons.left
        anchors.leftMargin: main.margin + 20*dpi
        y: 60*dpi + 4*main.margin
        model: formatlist
        open: formatbutton.checked ? 1 : 0
        onTargetTextChanged: {
            formatbutton.checked = false;
            sidebuttons.checked = false;
            video.setFormat(targetText);
        }

        Component.onCompleted: {
            setTarget(initial_format);
        }
    }

    SimpleList {
        id: qualities
        anchors.left: sidebuttons.left
        anchors.leftMargin: main.margin + 20*dpi
        y: 80*dpi + 5*main.margin
        model: qualitylist
        open: qualitybutton.checked ? 1 : 0

        onTargetTextChanged: {
            qualitybutton.checked = false;
            sidebuttons.checked = false;
            video.setQuality(targetText);
        }

        Component.onCompleted: {
            setTarget(initial_quality);
        }
    }


    Column {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: main.margin
        width: parent.width-2*main.margin
        spacing: main.margin

        SimpleButton {
            id: camera_settings
            width: 20*dpi
            height: 20*dpi
            enabled: false
            icon: "ion-aperture"
            explanation: "Options"
            visible: enabled

            onClicked: {
                video.showCameraProperties();
            }
        }

        SimpleButton {
            id: error_text
            width: 20*dpi
            height: 20*dpi
            enabled: false
            icon: "ion-alert-circled"
            iconcolor: "red"
            opacity: error_timer.running ? 1 : 0
            Behavior on opacity{NumberAnimation{duration: 100}}
            visible: opacity > 0
            checked: true
        }

        SimpleButton {
            width: 20*dpi
            height: 20*dpi
            enabled: false
            icon: "ion-alert-circled"
            iconcolor: "red"
            explanation: "Playback lagging " + (video.sleeptime > 0 ? "0 ms" : ((-video.sleeptime).toFixed(0) + " ms, frame skips possible"))
            opacity: video.sleeptime < 0 ? 1 : 0
            Behavior on opacity{NumberAnimation{duration: 100}}
            visible: opacity > 0
            checked: true
        }

        SimpleButton {
            width: 20*dpi
            height: 20*dpi
            enabled: false
            icon: "ion-information-circled"
            explanation: "Recorded: " + timestring(video.recordtime)
            opacity: video.recordtime > 0 ? 1 : 0
            Behavior on opacity{NumberAnimation{duration: 100}}
            visible: opacity > 0
            checked: true
        }

        Row {
            id: playrow
            width: parent.width-2*main.margin
            height: 20*(player.duration > 0 ? 1 : 0)*dpi
            spacing: main.margin/2

            visible: player.duration > 0

            Behavior on opacity{NumberAnimation{duration: 200}}

            SimpleButton {
                id: rewindbutton
                height: parent.height
                width: height
                icon: "ion-ios-skipbackward"
                disabled: player.duration <= 0
                onClicked: {
                    player.seek(0);
                }
            }

            SimpleButton {
                id: playbutton
                height: parent.height
                width: height
                icon: checked ? "ion-play" : "ion-pause"
                disabled: player.duration <= 0
                onClicked: {
                    checked = !checked;
                    if(checked)
                        player.pause();
                    else
                        player.play();
                }
            }

            Rectangle {
                width: playrow.width - playbutton.width - rewindbutton.width - 1.5*playrow.spacing
                height: parent.height
                radius: main.radius
                color: "white"
                opacity: 0.5

                MouseArea {
                    property bool skip_seek: false
                    anchors.fill: parent
                    onPressed: {
                        var fac = mouse.x/width;
                        if(fac < 0) fac = 0;
                        if(fac > 1) fac = 1;
                        if(!playbutton.checked)
                            player.pause();

                        player.seek(player.duration*fac);
                        skip_seek = true;
                    }
                    onMouseXChanged: {
                        if(!skip_seek) {
                            var fac = mouse.x/width;
                            if(fac < 0) fac = 0;
                            if(fac > 1) fac = 1;
                            player.seek(player.duration*fac);
                        }
                        skip_seek = false;
                    }

                    onReleased: {
                        if(!playbutton.checked) {
                            player.play();
                            timed_play.start();
                        }
                    }

                    Timer {
                        id: timed_play
                        interval: 100
                        onTriggered: {
                            if(player.playbackState != AVPlayer.PlayingState) {
                                player.play();
                                timed_play.start();
                            }
                        }
                    }
                }

                Rectangle {
                    height: parent.height
                    width: parent.width * player.position/player.duration
                    color: "white"
                    opacity: 1
                    radius: main.radius
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: main.margin
                    anchors.verticalCenter: parent.verticalCenter
                    text: timestring_ms(player.position)
                    color: "black"
                    font.pixelSize: parent.height*0.6
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: main.margin
                    anchors.verticalCenter: parent.verticalCenter
                    text: player.duration > 0 ? timestring_ms(player.duration) : ""
                    color: "black"
                    font.pixelSize: parent.height*0.6
                }
            }
        }
    }


    FileDialog {
        id: filedialog
        title: "Choose a video file"
        folder: shortcuts.movies
        onAccepted: {
            sources.setTarget(filedialog.fileUrls[0]);
        }
        onRejected: {
            sources.setTarget(sources.prevTargetText);
        }
        nameFilters: ["Video files (*.asf *.avi *.flv *.m4v *.mkv *.mov *.mp4)", "All files (*)"]
    }

    DropArea {
        id: droparea
        anchors.fill: parent
        property bool hover: false
        onEntered: {
            console.log(drag.urls);
            droptext.text = drag.urls.length > 0 ? "Play\n" + drag.urls[0] + "?" : "No file";
            droptext.horizontalAlignment = Text.AlignLeft;
            hover = true;
        }
        onExited: {
            droptext.text = "Drag a video file here\nor select an input."
            horizontalAlignment: Text.AlignHCenter
            hover = false;
        }
        onDropped: {
            if(drop.urls.length)
                sources.setTarget(drop.urls[0]);

            droptext.text = "Drag a video file here\nor select an input."
            horizontalAlignment: Text.AlignHCenter
            hover = false;
        }

        Text {
            id: droptext
            anchors.centerIn: parent
            color: "white"
            text: "Drag a video file here\nor select an input."
            horizontalAlignment: Text.AlignHCenter
            opacity: (sources.currentItem.text == "None" || parent.hover) && !sidebuttons.checked ? 0.5 : 0
            Behavior on opacity{NumberAnimation{duration: 100}}
        }
    }
}
