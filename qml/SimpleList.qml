import QtQuick 2.4
import "ionicons.js" as Ionicons

ListView {
    id: list
    property real open: 0
    Behavior on open {NumberAnimation{ easing.type: Easing.InOutQuad; duration: 100}}
    opacity: open
    width: open*parent.width
    height: open*Math.min(parent.height-y-20*dpi, count*20*dpi)
    property string prevTargetText: ""
    property string targetText: ""
    property bool options_open: false
    property string listOptionsForText: ""
    property var currentOptions: []

    function setTarget(str) {
        prevTargetText = targetText;
        targetText = str;

        for (var property in model) {
            if(basename(model[property]) == basename(targetText))
                currentIndex = property;
        }
    }

    property string currentText: ""
    property string prevText: ""
    property bool skipChange: false

    property string pendingSelection: ""

    function undo() {
        skipChange = true;
        currentText = prevText;
    }

    function basename(str) {
        if(str.indexOf(" - "))
            return str.split(" - ")[0];
        return str;
    }

    function humanreadable(str) {
        if(str.indexOf("#") == 0) {
            if(str.indexOf("?") >= 0)
                return str.split("#")[1].split("?")[0];
            else
                return str.split("#")[1]
        }
        if(str.indexOf("?") >= 0)
            str = str.split("?")[0];

        if(str.indexOf("feed:///") == 0) return str.split("feed:///")[1];
        if(str.indexOf("file:///") == 0) return str.split("file:///")[1];
        if(str.indexOf("dshow:///") == 0) return str.split("dshow:///")[1];
        if(str.indexOf("gdigrab:///") == 0) return str.split("gdigrab:///")[1];
        return str;
    }

    function basetext(str) {
        if(str.indexOf("?") >= 0)
            return str.split("?")[0];
        return str;
    }

    function defaultoption(str) {
        var options = optionslist(str);
        var def = "";

        options.forEach(function(opt) {
            if(def == "")
                def = opt;

            if(opt.indexOf("*") == 0) {
                def = opt;
                def.replace("*", "");
            }
        });
        return def;
    }

    function actiontext(str) {
        if(str.indexOf("?") >= 0) {
            str = str.replace("*", "");
            str = str.replace("!", "");
            return str.split("?")[1];
        }
        else
            return "";
    }

    function optionslist(str) {
        if(str.indexOf("?") >= 0)
            return str.split("?")[1].split(",");
        return [];
    }

    onModelChanged: {
        console.log("Model changed", model);
        var found = false;
        for (var property in model) {
            if(basename(model[property]) == basename(targetText)) {
                currentIndex = property;
                found = true;
            }
        }

        if(!found)
            currentIndex = 0;

        updateCurrentOptions();
    }

    function updateCurrentOptions() {
        for(var property in model) {
            var skip = false;
            for(var i=0; i<currentOptions.length; i++) {
                if(humanreadable(currentOptions[i]) == humanreadable(model[property]))
                    skip = true;
            }

            if(!skip) {
                if(model[property].indexOf("?*") >= 0 || model[property].indexOf("?!*") >= 0)
                    currentOptions.push(model[property]);
            }
        }
    }

    Component.onCompleted: {
        updateCurrentOptions();
    }

    delegate: Item {
        id: it
        width: list.width
        height: isTitle ? 30*dpi : (!isOption || isCurrentOption || listOptions) ? 20*dpi : 0
        clip: true

        property bool listOptions: humanreadable(text) == list.listOptionsForText

        property string text: String(modelData)
        property bool isTitle: text.indexOf("#") == 0
        property bool isOption: text.indexOf("?") >= 0
        property bool isCurrentOption: {list.currentOptions.indexOf(text) >= 0}
        property bool isFirstOption: text.indexOf("?!") >= 0

        MouseArea {
            enabled: list.opacity > 0 && !it.isTitle && (!it.listOptions || it.isFirstOption)
            anchors.fill: parent
            hoverEnabled: true
            property bool hover: false
            anchors.topMargin: it.isTitle ? 10 : 0

            onEntered: {hover = true;}
            onExited: {hover = false;}

            opacity: !listOptions || isFirstOption ? 1 : 0

            onClicked: {
                if(it.listOptions)
                    list.listOptionsForText = "";
                else if(!it.isTitle)
                    list.setTarget(it.text);
            }

            Text {
                id: mainText
                anchors.verticalCenter: parent.verticalCenter
                x: main.margin/2
                text: humanreadable(it.text)
                color: "white"
                font.pixelSize: parent.height*0.6
                opacity: parent.hover || list.currentIndex == index || listOptions ? 1 : 0.5
                font.bold: list.currentIndex == index || it.isTitle || listOptions
                style: Text.Outline
                styleColor: "black"
            }
        }

        MouseArea {
            enabled: list.opacity > 0 && (it.isTitle || it.isOption)
            width: enabled ? 200*dpi : 0
            height: enabled ? 20*dpi : 0
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: it.isTitle ? 10*dpi : 0
            anchors.leftMargin: mainText.contentWidth + 20*dpi

            hoverEnabled: true
            property bool hover: false

            onEntered: {hover = true;}
            onExited: {hover = false;}

            onClicked: {
                if(it.isTitle)
                    video.action(it.text);
                else {
                    if(it.listOptions) {
                        var newlist = [];
                        for(var i=0; i<list.currentOptions.length; i++) {
                            if(humanreadable(list.currentOptions[i]) == humanreadable(it.text)) {
                                newlist.push(it.text);
                            }
                            else {
                                newlist.push(list.currentOptions[i]);
                            }
                        }

                        list.currentOptions = newlist;

                        list.listOptionsForText = "";
                        list.setTarget(it.text);
                    }
                    else
                        list.listOptionsForText = humanreadable(it.text);
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                x: main.margin/2
                text: actiontext(it.text)
                color: "white"
                font.pixelSize: parent.height*0.6
                opacity: parent.hover || (it.listOptions && it.isCurrentOption) ? 1 : 0.5
                style: Text.Outline
                styleColor: "black"
            }
        }
    }
}
