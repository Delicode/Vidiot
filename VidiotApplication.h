#ifndef VIDIOTAPPLICATION_H
#define VIDIOTAPPLICATION_H

#include "predefines.h"

static QString eventToName(QEvent *ev)
{
    switch(ev->type()) {
    case 0: return "None";
    case 114: return "ActionAdded";
    case 113: return "ActionChanged";
    case 115: return "ActionRemoved";
    case 99: return "ActivationChange";
    case 121: return "ApplicationActivate";
    case 122: return "ApplicationDeactivate";
    case 36: return "ApplicationFontChange";
    case 37: return "ApplicationLayoutDirectionChange";
    case 38: return "ApplicationPaletteChange";
    case 214: return "ApplicationStateChange";
    case 35: return "ApplicationWindowIconChange";
    case 68: return "ChildAdded";
    case 69: return "ChildPolished";
    case 71: return "ChildRemoved";
    case 40: return "Clipboard";
    case 19: return "Close";
    case 200: return "CloseSoftwareInputPanel";
    case 178: return "ContentsRectChange";
    case 82: return "ContextMenu";
    case 183: return "CursorChange";
    case 52: return "DeferredDelete";
    case 60: return "DragEnter";
    case 62: return "DragLeave";
    case 61: return "DragMove";
    case 63: return "Drop";
    case 170: return "DynamicPropertyChange";
    case 98: return "EnabledChange";
    case 10: return "Enter";
    case 150: return "EnterEditFocus";
    case 124: return "EnterWhatsThisMode";
    case 206: return "Expose";
    case 116: return "FileOpen";
    case 8: return "FocusIn";
    case 9: return "FocusOut";
    case 23: return "FocusAboutToChange";
    case 97: return "FontChange";
    case 198: return "Gesture";
    case 202: return "GestureOverride";
    case 188: return "GrabKeyboard";
    case 186: return "GrabMouse";
    case 159: return "GraphicsSceneContextMenu";
    case 164: return "GraphicsSceneDragEnter";
    case 166: return "GraphicsSceneDragLeave";
    case 165: return "GraphicsSceneDragMove";
    case 167: return "GraphicsSceneDrop";
    case 163: return "GraphicsSceneHelp";
    case 160: return "GraphicsSceneHoverEnter";
    case 162: return "GraphicsSceneHoverLeave";
    case 161: return "GraphicsSceneHoverMove";
    case 158: return "GraphicsSceneMouseDoubleClick";
    case 155: return "GraphicsSceneMouseMove";
    case 156: return "GraphicsSceneMousePress";
    case 157: return "GraphicsSceneMouseRelease";
    case 182: return "GraphicsSceneMove";
    case 181: return "GraphicsSceneResize";
    case 168: return "GraphicsSceneWheel";
    case 18: return "Hide";
    case 27: return "HideToParent";
    case 127: return "HoverEnter";
    case 128: return "HoverLeave";
    case 129: return "HoverMove";
    case 96: return "IconDrag";
    case 101: return "IconTextChange";
    case 83: return "InputMethod";
    case 207: return "InputMethodQuery";
    case 169: return "KeyboardLayoutChange";
    case 6: return "KeyPress";
    case 7: return "KeyRelease";
    case 89: return "LanguageChange";
    case 90: return "LayoutDirectionChange";
    case 76: return "LayoutRequest";
    case 11: return "Leave";
    case 151: return "LeaveEditFocus";
    case 125: return "LeaveWhatsThisMode";
    case 88: return "LocaleChange";
    case 176: return "NonClientAreaMouseButtonDblClick";
    case 174: return "NonClientAreaMouseButtonPress";
    case 175: return "NonClientAreaMouseButtonRelease";
    case 173: return "NonClientAreaMouseMove";
    case 177: return "MacSizeChange";
    case 43: return "MetaCall";
    case 102: return "ModifiedChange";
    case 4: return "MouseButtonDblClick";
    case 2: return "MouseButtonPress";
    case 3: return "MouseButtonRelease";
    case 5: return "MouseMove";
    case 109: return "MouseTrackingChange";
    case 13: return "Move";
    case 197: return "NativeGesture";
    case 208: return "OrientationChange";
    case 12: return "Paint";
    case 39: return "PaletteChange";
    case 131: return "ParentAboutToChange";
    case 21: return "ParentChange";
    case 212: return "PlatformPanel";
    case 75: return "Polish";
    case 74: return "PolishRequest";
    case 123: return "QueryWhatsThis";
    case 106: return "ReadOnlyChange";
    case 199: return "RequestSoftwareInputPanel";
    case 14: return "Resize";
    case 204: return "ScrollPrepare";
    case 205: return "Scroll";
    case 117: return "Shortcut";
    case 51: return "ShortcutOverride";
    case 17: return "Show";
    case 26: return "ShowToParent";
    case 50: return "SockAct";
    case 192: return "StateMachineSignal";
    case 193: return "StateMachineWrapped";
    case 112: return "StatusTip";
    case 100: return "StyleChange";
    case 87: return "TabletMove";
    case 92: return "TabletPress";
    case 93: return "TabletRelease";
    case 94: return "OkRequest";
    case 171: return "TabletEnterProximity";
    case 172: return "TabletLeaveProximity";
    case 22: return "ThreadChange";
    case 1: return "Timer";
    case 120: return "ToolBarChange";
    case 110: return "ToolTip";
    case 184: return "ToolTipChange";
    case 194: return "TouchBegin";
    case 209: return "TouchCancel";
    case 196: return "TouchEnd";
    case 195: return "TouchUpdate";
    case 189: return "UngrabKeyboard";
    case 187: return "UngrabMouse";
    case 78: return "UpdateLater";
    case 77: return "UpdateRequest";
    case 111: return "WhatsThis";
    case 118: return "WhatsThisClicked";
    case 31: return "Wheel";
    case 132: return "WinEventAct";
    case 24: return "WindowActivate";
    case 103: return "WindowBlocked";
    case 25: return "WindowDeactivate";
    case 34: return "WindowIconChange";
    case 105: return "WindowStateChange";
    case 33: return "WindowTitleChange";
    case 104: return "WindowUnblocked";
    case 203: return "WinIdChange";
    case 126: return "ZOrderChange";
    default: return "unknown";
    }
}

class VidiotApplication : public QApplication
{
    Q_OBJECT
public:
    VidiotApplication(int &argc, char** argv);
    QHash<QString,QList<QString> > filterEvents;
    QHash<QString,QList<QString> > interestingEvents;

    // Uncomment to get massive loads of debug about events
#if 0
    virtual bool notify(QObject *ob, QEvent *ev) {

        QString classname = QString(ob->metaObject()->className());
        QString eventname = eventToName(ev);
        if (filterEvents.contains(classname) && filterEvents[classname].contains(eventname))
            return QApplication::notify(ob, ev);

        qDebug() << ob->metaObject()->className() << ", " << ob->objectName() << ", " << eventname;

        if (interestingEvents.contains(classname) && interestingEvents[classname].contains(eventname))
        {
            qDebug() << "interesting event " << ob->objectName();
        }

        return QApplication::notify(ob, ev);
    }
#endif
};

#endif // VIDIOTAPPLICATION_H
