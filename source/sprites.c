#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

typedef struct spriteAnimation
{
    C2D_Sprite* frames;
    size_t numFrames;
    size_t curIndex;
    u8 fps;
} sprite_Anim;

sprite_Anim createAnim(C2D_Sprite* sprites, size_t numFrames, u8 fps)
{
    sprite_Anim anim;
    
    anim.curIndex = 0;
    anim.fps = fps;
    anim.numFrames = numFrames;
    anim.frames = (C2D_Sprite*)malloc(numFrames * sizeof(C2D_Sprite));

    for (size_t i = 0; i < numFrames; i++)
    {
        anim.frames[i] = sprites[i];
    }

    return anim;
}

sprite_Anim* initSprites(C2D_SpriteSheet spriteSheet, size_t* lengthPtr)
{
    *lengthPtr = 16;

    C2D_Sprite curSprite[4];

    sprite_Anim* sprites = (sprite_Anim*)malloc(*lengthPtr * sizeof(sprite_Anim));

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 0); //Ground
    sprites[0] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 1); //Sun
    sprites[1] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 2); //Cloud
    sprites[2] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 3); //GameOver
    sprites[3] = createAnim(&curSprite[0], 1, 0);

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 4); //T-Rex Running
    }
    sprites[4] = createAnim(curSprite, 2, 20);

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 6); //T-Rex Ducking
    }
    sprites[5] = createAnim(curSprite, 2, 30);

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 8); //T-Rex Hit
    }
    sprites[6] = createAnim(curSprite, 2, 30);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 10); //Cactus1
    sprites[7] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 11); //Cactus2
    sprites[8] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 12); //Cactus3
    sprites[9] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 13); //Cactus4
    sprites[10] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 14); //Cactus Big 1
    sprites[11] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 15); //Cactus Big 2
    sprites[12] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 16); //Cactus Big 3
    sprites[13] = createAnim(&curSprite[0], 1, 0);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 17); //Cactus Large
    sprites[14] = createAnim(&curSprite[0], 1, 0);

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 18); //Bird
    }
    sprites[15] = createAnim(curSprite, 2, 30);

    return sprites;
}