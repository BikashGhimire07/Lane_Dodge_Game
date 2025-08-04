#pragma once

// Game-wide constants and externs
#pragma once



#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800

extern void display();
extern void timer(int value);

extern int score, highScore, lives;
extern bool gameOver;
extern float playerKmh, playerBaseSpeed, playerMaxSpeed;


