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

static sound jumpSfx = { "romfs:/jump.bin", NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT };
static sound scoreSfx = { "romfs:/score.bin", NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT };

static float rGroundLvl = 0.0f;
static float dGroundLvl = 0.0f;
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

void setCloudPos(sprite* cloudPtr)
{
    setSpritePos(cloudPtr, TOP_SCREEN_WIDTH + getRandom(1, TOP_SCREEN_WIDTH * 3), getRandom(minCloudLvl, maxCloudLvl));
}

void updateClouds(sprite* clouds)
{
    sprite* curCloudPtr;
    for (size_t i = 0; i < MAX_CLOUDS; i++)
    {
        curCloudPtr = &clouds[i];

        if (curCloudPtr->frames[curCloudPtr->curIndex].params.pos.x < -TOP_SCREEN_WIDTH)
        {
            setCloudPos(curCloudPtr);
        }

        moveSprite(curCloudPtr, xSpeed / 1.2f, 0);
    }
}

void playerRun(player* playerPtr)
{
    setPlayerPos(playerPtr, PLAYER_X, rGroundLvl);
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
                setPlayerPos(playerPtr, PLAYER_X, dGroundLvl);
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
            if (curSprite.frames[curSprite.curIndex].params.pos.y >= rGroundLvl) playerRun(playerPtr);
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
    init();

    C3D_RenderTarget* topPtr = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

    C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    if (!spriteSheet) svcBreak(USERBREAK_PANIC);

    sprite ground = initGround(spriteSheet);
    sprite groundExt = initGround(spriteSheet);
    setSpritePos(&ground, 0, SCREEN_HEIGHT - ground.hitbox.height);
    setSpritePos(&groundExt, ground.hitbox.width, SCREEN_HEIGHT - groundExt.hitbox.height);

    xSpeed = -5;

    sprite sun = initSun(spriteSheet);
    setSpritePos(&sun, TOP_SCREEN_WIDTH - sun.hitbox.width * 2, sun.hitbox.height / 2);

    sprite clouds[MAX_CLOUDS] = {};

    clouds[0] = initCloud(spriteSheet);

    minCloudLvl = clouds[0].hitbox.height;
    maxCloudLvl = SCREEN_HEIGHT / 3;

    setCloudPos(&clouds[0]);

    for (size_t i = 1; i < MAX_CLOUDS; i++)
    {
        clouds[i] = initCloud(spriteSheet);
        setCloudPos(&clouds[i]);
    }

    //sprite gameOver = initGameOver(spriteSheet);

    player player = initPlayer(spriteSheet);

    rGroundLvl = SCREEN_HEIGHT - player.sprites[STATE_RUNNING].hitbox.height - ground.hitbox.height / 2;
    dGroundLvl = SCREEN_HEIGHT - player.sprites[STATE_DUCKING].hitbox.height - ground.hitbox.height / 2;

    setPlayerPos(&player, PLAYER_X, rGroundLvl);

    //sprite* cacti = initCacti(spriteSheet);
    //sprite bird = initBird(spriteSheet);

    u32 frames = 0;

    sprite* curCloudPtr;
	while (aptMainLoop())
	{
        hidScanInput();

        kDown = hidKeysDown();
        kHeld = hidKeysHeld();
        kUp = hidKeysUp();

        if (kDown & KEY_START) break;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        
        frames++;

        if (frames % 600 == 0)
        {
            if (xSpeed > X_SPEED_MAX)
            {
                xSpeed -= 0.5f;
                audioPlay(&scoreSfx);
            }
        }

        updateGround(&ground, &groundExt);
        updateClouds(clouds);
        updatePlayer(&player);

        C2D_TargetClear(topPtr, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(topPtr);

        renderSprite(&sun, frames);

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

        renderSprite(&player.sprites[player.state], frames);
        
        C3D_FrameEnd(0);
	}

    C2D_SpriteSheetFree(spriteSheet);

    close();
	return 0;
}