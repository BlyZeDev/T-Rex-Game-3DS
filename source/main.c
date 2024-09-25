#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "game.h"

#define TOP_SCREEN_WIDTH  400
#define BOTTOM_SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define X_SPEED_MAX -10.0f
#define PLAYER_X 15.0f

static float rGroundLvl = 0.0f;
static float dGroundLvl = 0.0f;
static float xSpeed = 0.0f;
static u32 kDown = 0;
static u32 kHeld = 0;
static u32 kUp = 0;

void init()
{
    romfsInit();
	gfxInitDefault();
    hidInit();
	mcuHwcInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
}

void close()
{
    C2D_Fini();
    C3D_Fini();
	mcuHwcExit();
	hidExit();
	gfxExit();
    romfsExit();
}

void updateGround(sprite* groundPtr, sprite* groundExtPtr)
{
    if (groundPtr->frames[groundExtPtr->curIndex].params.pos.x < -groundPtr->hitbox.width)
    {
        setSpritePos(groundPtr, groundExtPtr->frames[groundExtPtr->curIndex].params.pos.x + groundExtPtr->hitbox.width, groundExtPtr->frames[0].params.pos.y);
    }
    else if (groundExtPtr->frames[groundExtPtr->curIndex].params.pos.x < -groundExtPtr->hitbox.width)
    {
        setSpritePos(groundExtPtr, groundPtr->frames[groundExtPtr->curIndex].params.pos.x + groundPtr->hitbox.width, groundPtr->frames[0].params.pos.y);
    }

    moveSprite(groundPtr, xSpeed, 0);
    moveSprite(groundExtPtr, xSpeed, 0);
}

void updatePlayer(player* playerPtr)
{
    switch (playerPtr->state)
    {
        case STATE_RUNNING:
            if (kDown & KEY_A)
            {
                playerPtr->state = STATE_JUMPING;
                playerPtr->jumpVelocity = JUMP_FORCE;
            }
            else if (kDown & KEY_B)
            {
                playerPtr->state = STATE_DUCKING;
                setPlayerPos(playerPtr, PLAYER_X, dGroundLvl);
            }
            break;

        case STATE_JUMPING:
            movePlayer(playerPtr, 0, playerPtr->jumpVelocity);
            playerPtr->jumpVelocity += GRAVITY;

            if (playerPtr->sprites[playerPtr->state].frames[playerPtr->sprites[playerPtr->state].curIndex].params.pos.y >= rGroundLvl)
            {
                setPlayerPos(playerPtr, PLAYER_X, rGroundLvl);
                playerPtr->jumpVelocity = 0.0f;
                playerPtr->state = STATE_RUNNING;
            }
            break;

        case STATE_DUCKING:
            if (kUp & KEY_B)
            {
                playerPtr->state = STATE_RUNNING;
                setPlayerPos(playerPtr, PLAYER_X, rGroundLvl);

                if (kDown & KEY_A)
                {
                    playerPtr->state = STATE_JUMPING;
                    playerPtr->jumpVelocity = JUMP_FORCE;
                }
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

    //sprite clound = initCloud(spriteSheet);
    //sprite gameOver = initGameOver(spriteSheet);

    player player = initPlayer(spriteSheet);

    rGroundLvl = SCREEN_HEIGHT - player.sprites[STATE_RUNNING].hitbox.height - ground.hitbox.height / 2;
    dGroundLvl = SCREEN_HEIGHT - player.sprites[STATE_DUCKING].hitbox.height - ground.hitbox.height / 2;

    setPlayerPos(&player, PLAYER_X, rGroundLvl);

    //sprite* cacti = initCacti(spriteSheet);
    //sprite bird = initBird(spriteSheet);

    u32 frames = 0;

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
            if (xSpeed > X_SPEED_MAX) xSpeed -= 0.5f;
        }

        updateGround(&ground, &groundExt);
        updatePlayer(&player);

        C2D_TargetClear(topPtr, C2D_Color32(0, 0, 0, 255));
		C2D_SceneBegin(topPtr);

        renderSprite(&ground, frames);
        renderSprite(&groundExt, frames);

        renderSprite(&sun, frames);

        renderSprite(&player.sprites[player.state], frames);
        
        C3D_FrameEnd(0);
	}

    C2D_SpriteSheetFree(spriteSheet);

    close();
	return 0;
}