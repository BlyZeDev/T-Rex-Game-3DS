#ifndef SPRITES_H
#define SPRITES_H

#define JUMP_FORCE -9.5f
#define GRAVITY 0.5f
#define GRAVITY_MULT 3.5f

typedef struct
{
    C2D_Sprite* frames;
    size_t numFrames;
    size_t curIndex;
    u8 fps;
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

player initPlayer(const C2D_SpriteSheet spriteSheet);

sprite initGround(const C2D_SpriteSheet spriteSheet);

sprite initCloud(const C2D_SpriteSheet spriteSheet);

sprite initGameOver(const C2D_SpriteSheet spriteSheet);

sprite* initCacti(const C2D_SpriteSheet spriteSheet, size_t* length);

sprite initBird(const C2D_SpriteSheet spriteSheet);

void setSpritePos(sprite* spritePtr, const float x, const float y);

void moveSprite(sprite* spritePtr, const float moveX, const float moveY);

void setPlayerPos(player* playerPtr, const float x, const float y);

void movePlayer(player* playerPtr, const float moveX, const float moveY);

void renderSprite(sprite* spritePtr, const u32 frames);

#endif