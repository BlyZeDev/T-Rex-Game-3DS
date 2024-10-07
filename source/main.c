#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>

#include "sprites.h"
#include "audio.h"
#include "input.h"
#include "fs.h"
#include "util.h"

#define TOP_SCREEN_WIDTH  400
#define BOTTOM_SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define JUMP_FORCE -10.0f
#define GRAVITY 0.55f
#define GRAVITY_MULT 4.0f
#define X_SPEED_MAX -10.0f
#define PLAYER_X_POS 15.0f
#define MAX_CLOUDS 5
#define CACTUS_MIN_SPACE TOP_SCREEN_WIDTH * 0.4f
#define CACTUS_MAX_SPACE TOP_SCREEN_WIDTH * 1.5f
#define BIRD_SPACE (CACTUS_MIN_SPACE + CACTUS_MAX_SPACE) * 0.35f
#define SCORE_TXT_BUFFER 11

typedef enum
{
    GSTATE_STOP,
    GSTATE_RUNNING,
    GSTATE_GAME_OVER
} gameState;

typedef struct
{
    u16 username[10];
    u32 zero;
    u32 ngWord;
} username;

static sound jumpSfx = { NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT, 8 };
static sound scoreSfx = { NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT, 9 };
static sound deathSfx = { NULL, 0, SOUND_FORMAT_16BIT | SOUND_ONE_SHOT, 10 };

static float groundLvl = 0.0f;
static float xSpeed = 0.0f;
static float minCloudLvl = 0.0f;
static float maxCloudLvl = 0.0f;
static float highBirdPos = 0.0f;
static float lowBirdPos = 0.0f;
static size_t lastCactusIndex = 0;

static gameState gState = GSTATE_RUNNING;

void init()
{
    osSetSpeedupEnable(true);
    romfsInit();
    initSd();
	gfxInitDefault();
    hidInit();
	mcuHwcInit();
    csndInit();
    cfguInit();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
}

void initSound()
{
    loadSound("romfs:/jump.bin", &jumpSfx);
    loadSound("romfs:/score.bin", &scoreSfx);
    loadSound("romfs:/death.bin", &deathSfx);
}

void close()
{
    C2D_Fini();
    C3D_Fini();
    cfguExit();
    csndExit();
	mcuHwcExit();
	hidExit();
	gfxExit();
    exitSd();
    romfsExit();
}

void getUsername(char* name, const size_t length)
{
    username nickname;

    CFGU_GetConfigInfoBlk2(sizeof(nickname), 0x000A0000, &nickname);

    ssize_t output = utf16_to_utf8((u8*)name, nickname.username, length - 1);
    name[output] = '\0';
}

void updateGround(sprite* groundPtr, sprite* groundExtPtr)
{
    float groundX = getPosX(*groundPtr);
    float groundExtX = getPosX(*groundExtPtr);

    float groundWidth = getWidth(*groundPtr);
    float groundExtWidth = getWidth(*groundExtPtr);

    if (groundX < -groundWidth)
    {
        setSpritePos(groundPtr, groundExtX + groundExtWidth, getPosY(*groundExtPtr));
    }
    else if (groundExtX < -groundExtWidth)
    {
        setSpritePos(groundExtPtr, groundX + groundWidth, getPosY(*groundPtr));
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

        if (getPosX(*curCloudPtr) < -TOP_SCREEN_WIDTH)
        {
            setSpritePos(curCloudPtr, TOP_SCREEN_WIDTH + getRandom(TOP_SCREEN_WIDTH / 2, TOP_SCREEN_WIDTH * 3), getRandom(minCloudLvl, maxCloudLvl));
        }

        moveSprite(curCloudPtr, xSpeed * 0.5f, 0);
    }
}

void updateCacti(sprite* cacti, const size_t length)
{
    sprite* curCactusPtr;
    for (size_t i = 0; i < length; i++)
    {
        curCactusPtr = &cacti[i];

        if (getPosX(*curCactusPtr) < -getWidth(*curCactusPtr))
        {
            setSpritePos(curCactusPtr,
                getPosX(cacti[lastCactusIndex]) + getWidth(cacti[lastCactusIndex]) + getRandom(CACTUS_MIN_SPACE, CACTUS_MAX_SPACE),
                groundLvl);

            lastCactusIndex++;
            if (lastCactusIndex == length) lastCactusIndex = 0;
        }

        moveSprite(curCactusPtr, xSpeed, 0);
    }
}

void updateBird(sprite* birdPtr, sprite* cacti, const size_t length)
{
    size_t secondLastCactusIndex = lastCactusIndex;
    if (secondLastCactusIndex == 0) secondLastCactusIndex = length - 1;


    float negBirdWidth = -getWidth(*birdPtr);
    if (getPosX(*birdPtr) < negBirdWidth)
    {
        float secondLastCactusPos = getPosX(cacti[secondLastCactusIndex]) + getWidth(cacti[secondLastCactusIndex]);

        if (getDiff(getPosX(cacti[lastCactusIndex]), secondLastCactusPos) > BIRD_SPACE)
        {
            setSpritePos(birdPtr,
                getMid(getPosX(cacti[lastCactusIndex]), secondLastCactusPos) + negBirdWidth,
                getRandom(0, 1) == 0 ? lowBirdPos : highBirdPos);
        }
    }

    moveSprite(birdPtr, xSpeed, 0);
}

void playerRun(player* playerPtr)
{
    setPlayerPos(playerPtr, PLAYER_X_POS, groundLvl);
    playerPtr->state = STATE_RUNNING;
    playerPtr->jumpVelocity = 0.0f;
}

void playerJump(player* playerPtr)
{
    playerPtr->jumpVelocity = JUMP_FORCE;
    playerPtr->state = STATE_JUMPING;
    playSound(&jumpSfx);
}

void playerDead(player* playerPtr)
{
    playerPtr->state = STATE_DEAD;
    gState = GSTATE_GAME_OVER;
    playSound(&deathSfx);
}

void playerCheckCollision(player* playerPtr, sprite* cacti, const size_t length, const sprite bird)
{
    sprite player = playerPtr->sprites[playerPtr->state];

    float posX = getPosX(player) + 2.75f;
    float posY = getPosY(player) - 5.0f;
    float width = getWidth(player) - 11.0f;
    float height = getHeight(player) - 5.0f;
    
    if (isInBounds(bird, posX, posY, width, height))
    {
        playerDead(playerPtr);
        return;
    }

    for (size_t i = 0; i < length; i++)
    {
        if (isInBounds(cacti[i], posX, posY, width, height))
        {
            playerDead(playerPtr);
            return;
        }
    }
}

void updatePlayer(player* playerPtr, sprite* cacti, const size_t length, const sprite bird)
{
    playerCheckCollision(playerPtr, cacti, length, bird);

    switch (playerPtr->state)
    {
        case STATE_RUNNING:
            if ((kDown | kHeld) & KEY_B)
            {
                playerPtr->state = STATE_DUCKING;
                setPlayerPos(playerPtr, PLAYER_X_POS, groundLvl);
            }
            else if ((kDown | kHeld) & KEY_A) playerJump(playerPtr);
            break;

        case STATE_JUMPING:
            movePlayer(playerPtr, 0, playerPtr->jumpVelocity);
            playerPtr->jumpVelocity += GRAVITY;

            if ((kDown | kHeld) & KEY_B)
            {
                playerPtr->jumpVelocity += GRAVITY * GRAVITY_MULT;
            }

            if (getPosY(playerPtr->sprites[playerPtr->state]) >= groundLvl) playerRun(playerPtr);
            break;

        case STATE_DUCKING:
            if (kUp & KEY_B)
            {
                playerRun(playerPtr);

                if ((kDown | kHeld) & KEY_A) playerJump(playerPtr);
            }
            break;

        case STATE_DEAD: break;
    }
}

int main(int argc, char** argv)
{
    init();
    initSound();

    const u32 TEXT_COLOR = C2D_Color32(255, 255, 255, 255);
    const u32 CLEAR_COLOR = C2D_Color32(0, 0, 0, 255);

    C3D_RenderTarget* topPtr = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottomPtr = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    
    C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    if (!spriteSheet) svcBreak(USERBREAK_PANIC);

    sprite ground = initGround(spriteSheet);
    sprite groundExt = initGround(spriteSheet);
    
    sprite* clouds = (sprite*)malloc(MAX_CLOUDS * sizeof(sprite));

    for (size_t i = 0; i < MAX_CLOUDS; i++)
    {
        clouds[i] = initCloud(spriteSheet);
    }

    minCloudLvl = getHeight(clouds[0]);
    maxCloudLvl = SCREEN_HEIGHT / 3.0f;

    sprite gameOver = initGameOver(spriteSheet);
    setSpritePos(&gameOver, roundf(BOTTOM_SCREEN_WIDTH / 2 - getWidth(gameOver) / 2), roundf(SCREEN_HEIGHT / 2 + getHeight(gameOver) / 2));

    player player = initPlayer(spriteSheet);

    groundLvl = SCREEN_HEIGHT - getHeight(ground) * 0.65f;

    size_t amountCacti = 0;
    sprite* cacti = initCacti(spriteSheet, &amountCacti);

    sprite bird = initBird(spriteSheet);

    highBirdPos = groundLvl - getHeight(player.sprites[STATE_DUCKING]) * 1.35f;
    lowBirdPos = groundLvl;

    char* usernameBuf = (char*)malloc(41 * sizeof(char));
    getUsername(usernameBuf, 41);

    char* highscoreTxtBuf = (char*)malloc(SCORE_TXT_BUFFER * sizeof(char));
    char* scoreTxtBuf = (char*)malloc(SCORE_TXT_BUFFER * sizeof(char));

    C2D_Font font = C2D_FontLoad("romfs:/font.bcfnt");
    C2D_TextBuf textBuf = C2D_TextBufNew(150);

    u32 frames;
    u32 score;
    C2D_Text nicknameTxt;
    C2D_Text highscoreTxt;
    C2D_Text scoreTxt;
    sprite* curSpritePtr;
	while (aptMainLoop())
	{
        srand(osGetTime());

        gState = GSTATE_RUNNING;

        frames = 0;
        score = 0;

        C2D_TextBufClear(textBuf);

        C2D_TextFontParse(&nicknameTxt, font, textBuf, usernameBuf);
        C2D_TextOptimize(&nicknameTxt);

        snprintf(highscoreTxtBuf, SCORE_TXT_BUFFER, "%05lu", readHighscore());
        C2D_TextFontParse(&highscoreTxt, font, textBuf, highscoreTxtBuf);
        C2D_TextOptimize(&highscoreTxt);

        snprintf(scoreTxtBuf, SCORE_TXT_BUFFER, "%05lu", score);
        C2D_TextFontParse(&scoreTxt, font, textBuf, scoreTxtBuf);
        C2D_TextOptimize(&scoreTxt);

        setSpritePos(&ground, 0, SCREEN_HEIGHT - getHeight(ground) * 0.5f);
        setSpritePos(&groundExt, getWidth(ground), SCREEN_HEIGHT - getHeight(groundExt) * 0.5f);
        
        xSpeed = -5;

        for (size_t i = 0; i < MAX_CLOUDS; i++)
        {
            setSpritePos(&clouds[i], getRandom(-TOP_SCREEN_WIDTH * 3, TOP_SCREEN_WIDTH * 3), getRandom(minCloudLvl, maxCloudLvl));
        }

        player.state = STATE_RUNNING;
        setPlayerPos(&player, PLAYER_X_POS, groundLvl);

        shuffleSprites(cacti, amountCacti);

        setSpritePos(&cacti[0], TOP_SCREEN_WIDTH, groundLvl);
        for (size_t i = 1; i < amountCacti; i++)
        {
            setSpritePos(&cacti[i],
                getPosX(cacti[i - 1]) + getWidth(cacti[i - 1]) + getRandom(CACTUS_MIN_SPACE, CACTUS_MAX_SPACE),
                groundLvl);
        }

        lastCactusIndex = amountCacti - 1;

        setSpritePos(&bird, -TOP_SCREEN_WIDTH - 1, 0);

        while (aptMainLoop())
        {
            updateInput();

            if (kDown & KEY_START)
            {
                gState = GSTATE_STOP;
                break;
            }

            if (gState == GSTATE_GAME_OVER)
            {
                if (isTouched(gameOver, touchPos) || (kDown & KEY_A)) break;
            }

            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

            frames++;

            if (gState == GSTATE_RUNNING)
            {
                if (frames % 6 == 0)
                {
                    score++;
                    C2D_TextBufClear(textBuf);
                    
                    C2D_TextFontParse(&nicknameTxt, font, textBuf, usernameBuf);
                    C2D_TextOptimize(&nicknameTxt);

                    C2D_TextFontParse(&highscoreTxt, font, textBuf, highscoreTxtBuf);
                    C2D_TextOptimize(&highscoreTxt);

                    snprintf(scoreTxtBuf, SCORE_TXT_BUFFER, "%05lu", score);
                    C2D_TextFontParse(&scoreTxt, font, textBuf, scoreTxtBuf);
                    C2D_TextOptimize(&scoreTxt);
                }

                if (frames % 600 == 0)
                {
                    if (xSpeed > X_SPEED_MAX)
                    {
                        xSpeed -= 0.5f;
                        playSound(&scoreSfx);
                    }
                }

                updateGround(&ground, &groundExt);
                updateClouds(clouds);

                updatePlayer(&player, cacti, amountCacti, bird);
                
                updateCacti(cacti, amountCacti);
                updateBird(&bird, cacti, amountCacti);
            }

            C2D_TargetClear(bottomPtr, CLEAR_COLOR);
            C2D_SceneBegin(bottomPtr);

            if (gState == GSTATE_GAME_OVER)
            {
                renderSprite(&gameOver, frames);
            }

            C2D_TargetClear(topPtr, CLEAR_COLOR);
            C2D_SceneBegin(topPtr);

            renderSprite(&ground, frames);
            renderSprite(&groundExt, frames);

            for (size_t i = 0; i < MAX_CLOUDS; i++)
            {
                curSpritePtr = &clouds[i];

                if (isInRange(getPosX(*curSpritePtr), -getWidth(*curSpritePtr), TOP_SCREEN_WIDTH))
                {
                    renderSprite(curSpritePtr, frames);
                }
            }

            C2D_DrawText(&nicknameTxt, C2D_AlignRight | C2D_WithColor, TOP_SCREEN_WIDTH * 0.675f, 0.0f, 0.0f, 0.5f, 0.5f, TEXT_COLOR);
            C2D_DrawText(&highscoreTxt, C2D_AlignRight | C2D_WithColor, TOP_SCREEN_WIDTH * 0.85f, 0.0f, 0.0f, 0.5f, 0.5f, TEXT_COLOR);
            C2D_DrawText(&scoreTxt, C2D_AlignRight | C2D_WithColor, TOP_SCREEN_WIDTH, 0.0f, 0.0f, 0.5f, 0.5f, TEXT_COLOR);

            for (size_t i = 0; i < amountCacti; i++)
            {
                curSpritePtr = &cacti[i];

                if (isInRange(getPosX(*curSpritePtr), -getWidth(*curSpritePtr), TOP_SCREEN_WIDTH))
                {
                    renderSprite(curSpritePtr, frames);
                }
            }

            if (isInRange(getPosX(bird), -getWidth(bird), TOP_SCREEN_WIDTH)) renderSprite(&bird, frames);
            
            renderSprite(&player.sprites[player.state], frames);

            C3D_FrameEnd(0);
        }

        if (readHighscore() < score)
        {
            writeHighscore(score);
        }

        if (gState == GSTATE_STOP) break;
	}

    free(usernameBuf);
    free(highscoreTxtBuf);
    free(scoreTxtBuf);
    free(clouds);
    free(cacti);
    C2D_SpriteSheetFree(spriteSheet);
    C3D_RenderTargetDelete(topPtr);
    C3D_RenderTargetDelete(bottomPtr);

    close();
	return 0;
}