#include "macwindowinitializer.h"

#include <AppKit/AppKit.h>
#include <AppKit/NSWindow.h>

MacWindowInitializer::MacWindowInitializer()
{
}

void MacWindowInitializer::setWindowFlags(QWidget *widget, bool ignore_mouse)
{
    NSView *view = (NSView*)widget->winId();
    NSWindow *window = [view window];
    if(ignore_mouse) {
        [window setIgnoresMouseEvents:YES];
    }
    [window setHasShadow:NO];
    [window setLevel:NSScreenSaverWindowLevel];
    [window setCollectionBehavior:
        NSWindowCollectionBehaviorFullScreenAuxiliary
        |NSWindowCollectionBehaviorStationary
        |NSWindowCollectionBehaviorIgnoresCycle
        |NSWindowCollectionBehaviorMoveToActiveSpace
        ];
}

void MacWindowInitializer::setApplicationAsBackground()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyProhibited];
}

void MacWindowInitializer::setApplicationAsUIElement()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
}

void MacWindowInitializer::setFront()
{
    ProcessSerialNumber psn;
    OSErr err = GetCurrentProcess(&psn);
    if (err == noErr)
    {
        SetFrontProcess(&psn);
    }
}

