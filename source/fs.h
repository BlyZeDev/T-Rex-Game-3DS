#ifndef FS_H
#define FS_H

void initSd();

u32 readHighscore();

void writeHighscore(u32 value);

void exitSd();

#endif