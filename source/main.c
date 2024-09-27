#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "game.h"
#include "audio.h"

#define TOP_SCREEN_WIDTH  400
#define BOTTOM_SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define X_SPEED_MAX -10.0f
#define PLAYER_X 15.0f
#define MAX_CLOUDS 5
#define CACTUS_MIN_SPACE TOP_SCREEN_WIDTH / 2.0f
#define CACTUS_MAX_SPACE TOP_SCREEN_WIDTH * 2.0f

static sound jumpSfx = { "romfs:/jump.bin", NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT };
static sound scoreSfx = { "romfs:/score.bin", NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT };

static float groundLvl = 0.0f;
static float xSpeed = 0.0f;
static float minCloudLvl = 0.0f;
static float maxCloudLvl = 0.0f;
static u32 kDown = 0;
static u32 kHeld = 0;
static u32 kUp = 0;

void init()
{
    romfsInit();
	gfxInitDefault();
    hidInit();
	mcuHwcInit();
    csndInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
}

void close()
{
    C2D_Fini();
    C3D_Fini();
    csndExit();
	mcuHwcExit();
	hidExit();
	gfxExit();
    romfsExit();
}

int getRandom(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void updateGround(sprite* groundPtr, sprite* groundExtPtr)
{
    if (groundPtr->frames[groundExtPtr->curIndex].params.pos.x < -groundPtr->hitbox.width)
    {
        setSpritePos(groundPtr,
            groundExtPtr->frames[groundExtPtr->curIndex].params.pos.x + groundExtPtr->hitbox.width,
            groundExtPtr->frames[groundExtPtr->curIndex].params.pos.y);
    }
    else if (groundExtPtr->frames[groundExtPtr->curIndex].params.pos.x < -groundExtPtr->hitbox.width)
    {
        setSpritePos(groundExtPtr,
            groundPtr->frames[groundExtPtr->curIndex].params.pos.x + groundPtr->hitbox.width,
            groundPtr->frames[groundPtr->curIndex].params.pos.y);
    }

    moveSprite(groundPtr, xSpeed, 0);
    moveSprite(groundExtPtr, xSpeed, 0);
}

void updateClouds(sprite* clouds)
{
    sprite* curCloudPtr;
    for (size_t i = 0; i < MAX_CLOUDS; i++)
    {
        curCloudPtr = &clouds[i];

        if (curCloudPtr->frames[curCloudPtr->curIndex].params.pos.x < -TOP_SCREEN_WIDTH)
        {
            setSpritePos(curCloudPtr, TOP_SCREEN_WIDTH + getRandom(TOP_SCREEN_WIDTH / 2, TOP_SCREEN_WIDTH * 3), getRandom(minCloudLvl, maxCloudLvl));
        }

        moveSprite(curCloudPtr, xSpeed / 1.35f, 0);
    }
}

void updateCacti(sprite* cacti, size_t length)
{
    //Update cacti
}

void playerRun(player* playerPtr)
{
    setPlayerPos(playerPtr, PLAYER_X, groundLvl);
    playerPtr->state = STATE_RUNNING;
    playerPtr->jumpVelocity = 0.0f;
}

void playerJump(player* playerPtr)
{
    playerPtr->jumpVelocity = JUMP_FORCE;
    playerPtr->state = STATE_JUMPING;
    audioPlay(&jumpSfx);
}

void updatePlayer(player* playerPtr)
{
    switch (playerPtr->state)
    {
        case STATE_RUNNING:
            if ((kDown | kHeld) & KEY_B)
            {
                playerPtr->state = STATE_DUCKING;
                setPlayerPos(playerPtr, PLAYER_X, groundLvl);
            }
            else if ((kDown | kHeld) & KEY_A) playerJump(playerPtr);
            break;

        case STATE_JUMPING:
            movePlayer(playerPtr, 0, playerPtr->jumpVelocity);
            playerPtr->jumpVelocity += GRAVITY;

            if ((kDown | kHeld) & KEY_B)
            {
                playerPtr->jumpVelocity += GRAVITY * 4;
            }

            sprite curSprite = playerPtr->sprites[playerPtr->state];
            if (curSprite.frames[curSprite.curIndex].params.pos.y >= groundLvl) playerRun(playerPtr);
            break;

        case STATE_DUCKING:
            if (kUp & KEY_B)
            {
                playerRun(playerPtr);

                if ((kDown | kHeld) & KEY_A) playerJump(playerPtr);
            }
            break;
    }
}

int main(int argc, char **argv)
{
    osSetSpeedupEnable(false);
    init();

    C3D_RenderTarget* topPtr = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    
    C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    if (!spriteSheet) svcBreak(USERBREAK_PANIC);

    sprite ground = initGround(spriteSheet);
    sprite groundExt = initGround(spriteSheet);
    setSpritePos(&ground, 0, SCREEN_HEIGHT - ground.hitbox.height * 0.5f);
    setSpritePos(&groundExt, ground.hitbox.width, SCREEN_HEIGHT - groundExt.hitbox.height * 0.5f);
    
    xSpeed = -5;

    sprite* clouds = (sprite*)malloc(MAX_CLOUDS * sizeof(sprite));

    clouds[0] = initCloud(spriteSheet);

    minCloudLvl = clouds[0].hitbox.height;
    maxCloudLvl = SCREEN_HEIGHT / 2.5f;

    setSpritePos(&clouds[0], getRandom(-TOP_SCREEN_WIDTH * 3, TOP_SCREEN_WIDTH * 3), getRandom(minCloudLvl, maxCloudLvl));

    for (size_t i = 1; i < MAX_CLOUDS; i++)
    {
        clouds[i] = initCloud(spriteSheet);
        setSpritePos(&clouds[i], getRandom(-TOP_SCREEN_WIDTH * 3, TOP_SCREEN_WIDTH * 3), getRandom(minCloudLvl, maxCloudLvl));
    }

    //sprite gameOver = initGameOver(spriteSheet);

    player player = initPlayer(spriteSheet);

    groundLvl = SCREEN_HEIGHT - ground.hitbox.height * 0.65f;

    setPlayerPos(&player, PLAYER_X, groundLvl);

    size_t amountCacti = 0;
    sprite* cacti = initCacti(spriteSheet, &amountCacti);

    //Set cacti positions

    //sprite bird = initBird(spriteSheet);

    u32 frames;
    sprite* curCloudPtr;
    sprite* curCactusPtr;
	while (aptMainLoop())
	{
        hidScanInput();

        kDown = hidKeysDown();
        kHeld = hidKeysHeld();
        kUp = hidKeysUp();

        if (kDown & KEY_START) break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
        frames = C3D_FrameCounter(0);

        if (frames % 600 == 0)
        {
            if (xSpeed > X_SPEED_MAX)
            {
                xSpeed -= 0.5f;
                audioStop(&jumpSfx);
                audioPlay(&scoreSfx);
            }
        }

        updateGround(&ground, &groundExt);
        updateClouds(clouds);
        updateCacti(cacti, amountCacti);
        updatePlayer(&player);

        C2D_TargetClear(topPtr, C2D_Color32(0, 0, 0, 255));
        C2D_SceneBegin(topPtr);

        renderSprite(&ground, frames);
        renderSprite(&groundExt, frames);

        for (size_t i = 0; i < MAX_CLOUDS; i++)
        {
            curCloudPtr = &clouds[i];

            if (curCloudPtr->frames[curCloudPtr->curIndex].params.pos.x >= -curCloudPtr->hitbox.width
                && curCloudPtr->frames[curCloudPtr->curIndex].params.pos.x <= TOP_SCREEN_WIDTH)
            {
                renderSprite(curCloudPtr, frames);
            }
        }

        for (size_t i = 0; i < amountCacti; i++)
        {
            curCactusPtr = &cacti[i];

            if (curCactusPtr->frames[curCactusPtr->curIndex].params.pos.x >= -curCactusPtr->hitbox.width
                && curCactusPtr->frames[curCactusPtr->curIndex].params.pos.x <= TOP_SCREEN_WIDTH)
            {
                renderSprite(curCactusPtr, frames);
            }
        }
        
        renderSprite(&player.sprites[player.state], frames);

        C3D_FrameEnd(0);
	}

    C2D_SpriteSheetFree(spriteSheet);

    close();
	return 0;
}