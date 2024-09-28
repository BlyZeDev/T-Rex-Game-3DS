#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "sprites.h"

float getPosX(const sprite spr)
{
    return spr.frames[spr.curIndex].params.pos.x;
}

float getPosY(const sprite spr)
{
    return spr.frames[spr.curIndex].params.pos.y;
}

float getWidth(const sprite spr)
{
    return spr.frames[spr.curIndex].params.pos.w;
}

float getHeight(const sprite spr)
{
    return spr.frames[spr.curIndex].params.pos.h;
}

float getMid(const float num1, const float num2)
{
    return (num1 + num2) / 2.0f;
}

float getDiff(const float num1, const float num2)
{
    return fabsf(num1 - num2);
}

bool isInRange(const float pos, const float start, const float end)
{
    return pos >= start && pos <= end;
}

bool isColliding(const sprite spr1, const sprite spr2)
{
    return !(getPosX(spr1) + getWidth(spr1) <= getPosX(spr2)
        || getPosX(spr2) + getWidth(spr2) <= getPosX(spr1)
        || getPosY(spr1) + getHeight(spr1) <= getPosY(spr2)
        || getPosY(spr2) + getHeight(spr2) <= getPosY(spr1));
}

int getRandom(const int min, const int max)
{
    return rand() % (max - min + 1) + min;
}