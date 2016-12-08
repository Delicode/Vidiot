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
