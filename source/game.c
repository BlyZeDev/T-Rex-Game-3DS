#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

typedef struct
{
    u16 width;
    u16 height;
} hitbox;

typedef struct
{
    C2D_Sprite* frames;
    size_t numFrames;
    size_t curIndex;
    u8 fps;

    hitbox hitbox;
} sprite;

typedef enum
{
    STATE_RUNNING,
    STATE_JUMPING,
    STATE_DUCKING,
    STATE_DEAD
} playerState;

typedef struct
{
    playerState state;
    sprite* sprites;
    size_t spriteCount;

    float jumpVelocity;
} player;

void updateFrames(sprite* spritePtr, u32 frames)
{
    if (spritePtr->fps > 0)
    {
        if (frames % spritePtr->fps == 0)
        {
            spritePtr->curIndex++;

            if (spritePtr->curIndex > spritePtr->numFrames - 1) spritePtr->curIndex = 0;
        }
    }
}

sprite initSprite(C2D_Sprite* sprites, size_t numFrames, u8 fps)
{
    sprite anim;
    
    anim.curIndex = 0;
    anim.fps = fps;
    anim.numFrames = numFrames;
    anim.frames = (C2D_Sprite*)malloc(numFrames * sizeof(C2D_Sprite));

    hitbox hb = { 0, 0 };
    for (size_t i = 0; i < numFrames; i++)
    {
        anim.frames[i] = sprites[i];

        C2D_SpriteSetCenter(&anim.frames[i], 0.0f, 1.0f);

        if (anim.frames[i].image.subtex->width > hb.width) hb.width = anim.frames[i].image.subtex->width;
        if (anim.frames[i].image.subtex->height > hb.height) hb.height = anim.frames[i].image.subtex->height;
    }

    anim.hitbox = hb;

    return anim;
}

player initPlayer(C2D_SpriteSheet spriteSheet)
{
    const size_t spriteCount = 4;

    player player;
    C2D_Sprite* curSprite = (C2D_Sprite*)malloc(2 * sizeof(C2D_Sprite));
    
    player.state = STATE_RUNNING;

    player.sprites = (sprite*)malloc(spriteCount * sizeof(sprite));

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 3); //T-Rex Running
    }
    player.sprites[0] = initSprite(curSprite, 2, 20);

    C2D_SpriteFromSheet(&curSprite[0], spriteSheet, 5);
    player.sprites[1] = initSprite(&curSprite[0], 1, 0);

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 6); //T-Rex Ducking
    }
    player.sprites[2] = initSprite(curSprite, 2, 30);

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 8); //T-Rex Dead
    }
    player.sprites[3] = initSprite(curSprite, 2, 60);

    player.spriteCount = spriteCount;
    player.jumpVelocity = 0.0f;

    free(curSprite);

    return player;
}

sprite initGround(C2D_SpriteSheet spriteSheet)
{
    C2D_Sprite curSprite;

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 0);
    return initSprite(&curSprite, 1, 0);
}

sprite initCloud(C2D_SpriteSheet spriteSheet)
{
    C2D_Sprite curSprite;

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 1);
    return initSprite(&curSprite, 1, 0);
}

sprite initGameOver(C2D_SpriteSheet spriteSheet)
{
    C2D_Sprite curSprite;

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 2);
    return initSprite(&curSprite, 1, 0);
}

sprite* initCacti(C2D_SpriteSheet spriteSheet, size_t* length)
{
    C2D_Sprite curSprite;
    *length = 8;
    sprite* sprites = (sprite*)malloc(*length * sizeof(sprite));

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 10); //Cactus1
    sprites[0] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 11); //Cactus2
    sprites[1] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 12); //Cactus3
    sprites[2] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 13); //Cactus4
    sprites[3] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 14); //Cactus Big 1
    sprites[4] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 15); //Cactus Big 2
    sprites[5] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 16); //Cactus Big 3
    sprites[6] = initSprite(&curSprite, 1, 0);

    C2D_SpriteFromSheet(&curSprite, spriteSheet, 17); //Cactus Large
    sprites[7] = initSprite(&curSprite, 1, 0);

    return sprites;
}

sprite initBird(C2D_SpriteSheet spriteSheet)
{
    C2D_Sprite* curSprite = (C2D_Sprite*)malloc(2 * sizeof(C2D_Sprite));

    for (int i = 0; i < 2; i++)
    {
        C2D_SpriteFromSheet(&curSprite[i], spriteSheet, i + 18);
    }

    sprite bird = initSprite(curSprite, 2, 30);

    free(curSprite);

    return bird;
}

void setSpritePos(sprite* spritePtr, float x, float y)
{
    for (size_t i = 0; i < spritePtr->numFrames; i++)
    {
        C2D_SpriteSetPos(&spritePtr->frames[i], x, y);
    }
}

void moveSprite(sprite* spritePtr, float moveX, float moveY)
{
    for (size_t i = 0; i < spritePtr->numFrames; i++)
    {
        C2D_SpriteMove(&spritePtr->frames[i], moveX, moveY);
    }
}

void setPlayerPos(player* playerPtr, float x, float y)
{
    for (size_t i = 0; i < playerPtr->spriteCount; i++)
    {
        setSpritePos(&playerPtr->sprites[i], x, y);
    }
}

void movePlayer(player* playerPtr, float moveX, float moveY)
{
    for (size_t i = 0; i < playerPtr->spriteCount; i++)
    {
        moveSprite(&playerPtr->sprites[i], moveX, moveY);
    }
}

void renderSprite(sprite* spritePtr, u32 frames)
{
    updateFrames(spritePtr, frames);

    C2D_DrawSprite(&spritePtr->frames[spritePtr->curIndex]);
}