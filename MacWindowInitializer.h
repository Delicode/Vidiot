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

#ifndef MACWINDOWINITIALIZER_H
#define MACWINDOWINITIALIZER_H

#include "predefines.h"
#include <mach/port.h>

class MacWindowInitializer
{
public:
    MacWindowInitializer();

    static void setWindowFlags(QWidget *widget, bool ignore_mouse = true);

    static void setApplicationAsBackground();

    static void setApplicationAsUIElement();
    static void setFront();
};

#endif // MACWINDOWINITIALIZER_H
