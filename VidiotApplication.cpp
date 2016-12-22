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
    //interestingEvents["QObject"].push_back("ThreadChange");
    interestingEvents["QObject"].push_back("MetaCall");
}
