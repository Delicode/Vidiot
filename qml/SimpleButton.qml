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
import "ionicons.js" as Ionicons

MouseArea {
    id: button
    enabled: parent.visible && visible && !disabled
    visible: parent.opacity > 0 && opacity > 0
    hoverEnabled: true

    property bool disabled: false
    property string text: ""
    property bool checked: false
    property string icon: ""
    property bool hover: false
    property string explanation: ""
    property color color: "white"
    property color iconcolor: color
    property color textcolor: color

    onEntered: {hover = true;}
    onExited: {hover = false;}

    opacity: disabled ? 0.5 : 1


    Item {
        id: iconarea
        height: parent.height
        width: height

        Text {
            id: txt
            anchors.centerIn: parent
            font.pixelSize: parent.height*0.8
            font.family: ionicons.name
            text: button.icon == "" ? button.text : Ionicons.getIcon(button.icon);
            color: button.iconcolor
            style: Text.Outline
            styleColor: "black"
            opacity: button.hover ? 1 : 0.5
        }
    }
    Text {
        anchors.verticalCenter: iconarea.verticalCenter
        anchors.left: iconarea.right
        anchors.leftMargin: main.margin/2
        text: explanation
        opacity: hover ? 1 : checked ? 0.5 : 0.2
        font.pixelSize: parent.height*0.8
        color: button.textcolor
        style: Text.Outline
        styleColor: "black"
    }
}
