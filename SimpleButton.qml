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
