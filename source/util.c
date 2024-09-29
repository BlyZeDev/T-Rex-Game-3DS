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
    return !(x + width <= getPosX(spr) || getPosX(spr) + getWidth(spr) <= x
        || y + height <= getPosY(spr) || getPosY(spr) + getHeight(spr) <= y);
}

int getRandom(const int min, const int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void swap(size_t* ptr1, size_t* ptr2)
{
    size_t temp = *ptr1;
    *ptr1 = *ptr2;
    *ptr2 = temp;
}

void shuffleArray(size_t* arr, size_t length)
{
    size_t temp;
    for (size_t i = length - 1; i > 0; i--)
    {
        temp = getRandom(0, length - 1);
 
        swap(&arr[i], &arr[temp]);
    }
}