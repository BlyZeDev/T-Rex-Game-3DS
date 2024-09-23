#ifndef SPRITES_H
#define SPRITES_H

typedef struct spriteAnimation
{
    C2D_Sprite* frames;
    size_t numFrames;
    size_t curIndex;
    u8 fps;
} sprite_Anim;

sprite_Anim* initSprites(C2D_SpriteSheet spriteSheet, size_t* lengthPtr);

#endif