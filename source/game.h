#ifndef GAME_H
#define GAME_H

#define JUMP_FORCE -10.0f
#define GRAVITY 0.5f

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

player initPlayer(C2D_SpriteSheet spriteSheet);

sprite initGround(C2D_SpriteSheet spriteSheet);

sprite initCloud(C2D_SpriteSheet spriteSheet);

sprite initGameOver(C2D_SpriteSheet spriteSheet);

sprite* initCacti(C2D_SpriteSheet spriteSheet, size_t* length);

sprite initBird(C2D_SpriteSheet spriteSheet);

void setSpritePos(sprite* spritePtr, float x, float y);

void moveSprite(sprite* spritePtr, float moveX, float moveY);

void setPlayerPos(player* playerPtr, float x, float y);

void movePlayer(player* playerPtr, float moveX, float moveY);

void renderSprite(sprite* spritePtr, u32 frames);

#endif