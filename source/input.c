#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

u32 kDown = 0;
u32 kHeld = 0;
u32 kUp = 0;

touchPosition touchPos;

void updateInput()
{
    hidScanInput();

    kDown = hidKeysDown() | hidKeysDownRepeat();
    kHeld = hidKeysHeld();
    kUp = hidKeysUp();

    hidTouchRead(&touchPos);
}