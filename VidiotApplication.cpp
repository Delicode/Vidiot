#include "VidiotApplication.h"

VidiotApplication::VidiotApplication(int& argc, char** argv) : QApplication(argc, argv)
{
    // Use this hash to make debugging the event spam easier
    filterEvents["QQmlDebugServerImpl"].push_back("MetaCall");
    filterEvents["QEventDispatcherWin32"].push_back("Timer");
    filterEvents["QTimer"].push_back("Timer");
    filterEvents["VideoView_QML_103"].push_back("MetaCall");
    filterEvents["QQuickView"].push_back("UpdateRequest");
    filterEvents["QQuickView"].push_back("Timer");
    filterEvents["QWindowsGuiEventDispatcher"].push_back("Timer");
    filterEvents["QQuickAnimatorControllerGuiThreadEntity"].push_back("MetaCall");

    // Use this to break on specific events or something like that
    interestingEvents["QObject"].push_back("ThreadChange");
}
