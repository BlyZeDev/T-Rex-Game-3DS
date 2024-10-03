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

bool isInBounds(const sprite spr, const float x, const float y, const float width, const float height)
{
    return getPosX(spr) + getWidth(spr) >= x && getPosX(spr) <= x + width
        && getPosY(spr) >= y - height && getPosY(spr) - getHeight(spr) <= y;
}

bool isTouched(const sprite spr, const touchPosition touchPos)
{
    return isInBounds(spr, touchPos.px, touchPos.py, 0, 0);
}

int getRandom(const int min, const int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void swap(sprite* sprPtr1, sprite* sprPtr2)
{
    sprite temp = *sprPtr1;
    *sprPtr1 = *sprPtr2;
    *sprPtr2 = temp;
}

void shuffleSprites(sprite* arr, size_t length)
{
    size_t temp;
    for (size_t i = length - 1; i > 0; i--)
    {
        temp = getRandom(0, length - 1);
 
        swap(&arr[i], &arr[temp]);
    }
}