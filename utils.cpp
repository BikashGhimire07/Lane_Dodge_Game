
// =================== Includes ===================
#include <windows.h>
#include <mmsystem.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <GL/glut.h>
#pragma comment(lib, "winmm.lib")
#include "constants.h"
#include "utils.h"
#include "game.h"
#include "player.h"
#include "vehicle.h"
#include "road.h"
#include "hud.h"
#include "input.h"

// Ensure GLUT mouse constants are available before mouseMenu
#ifndef GLUT_LEFT_BUTTON
#define GLUT_LEFT_BUTTON 0
#endif
#ifndef GLUT_DOWN
#define GLUT_DOWN 0
#endif

// =================== Global Variables ===================
bool normalMusicPlaying = false;
bool crashMusicPlaying = false;

// =================== Function Definitions ===================

// Audio helper functions for MP3 playback
void playMP3(const char* alias, const char* filename, bool loop) {
    char cmd[256];
    sprintf(cmd, "open \"%s\" type mpegvideo alias %s", filename, alias);
    mciSendString(cmd, NULL, 0, NULL);
    sprintf(cmd, "play %s%s", alias, loop ? " repeat" : "");
    mciSendString(cmd, NULL, 0, NULL);
}

void stopMP3(const char* alias) {
    char cmd[64];
    sprintf(cmd, "stop %s", alias);
    mciSendString(cmd, NULL, 0, NULL);
    sprintf(cmd, "close %s", alias);
    mciSendString(cmd, NULL, 0, NULL);
}

// Audio state (should be extern in header, defined in game.cpp)
extern bool normalMusicPlaying;
extern bool crashMusicPlaying;
extern bool playerCrashed;

void handleAudio() {
    if (playerCrashed) {
        if (normalMusicPlaying) {
            stopMP3("normal");
            normalMusicPlaying = false;
        }
        if (!crashMusicPlaying) {
            playMP3("crash", "audio/crashed.mp3", false);
            crashMusicPlaying = true;
        }
    } else {
        if (!normalMusicPlaying) {
            playMP3("normal", "audio/normal.mp3", true);
            normalMusicPlaying = true;
        }
        if (crashMusicPlaying) {
            stopMP3("crash");
            crashMusicPlaying = false;
        }
    }
}

// Mouse handler for menu buttons
extern bool showMenu;
extern bool gameOver;
extern int playerLane;
extern int playerLaneOffset;
extern float playerY;
extern bool playerVisible;
extern int score;
extern int lives;
extern float playerKmh;
extern float playerBaseSpeed;
extern float playerMaxSpeed;
extern int activeOppositeCars;
extern void initOppositeCars();
extern void initLaneDividers();

void mouseMenu(int button, int state, int x, int y) {
    if (!showMenu && !gameOver) return;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    // Convert y to OpenGL coordinates (origin at bottom left)
    int oglY = SCREEN_HEIGHT - y;
    // Button positions must match those in drawMenu()
    float btnW = 120, btnH = 40;
    float playBtnX = SCREEN_WIDTH/2 - btnW - 20, playBtnY = SCREEN_HEIGHT/2 - 40;
    float quitBtnX = SCREEN_WIDTH/2 + 20, quitBtnY = playBtnY;
    // Check Play button
    if (x >= playBtnX && x <= playBtnX + btnW && oglY >= playBtnY && oglY <= playBtnY + btnH) {
        // Start or restart game
        showMenu = false;
        if (gameOver) {
            // Reset all game state for restart
            playerLane = 1;
            playerLaneOffset = 2;
            playerY = PLAYER_START_Y;
            playerCrashed = false;
            playerVisible = true;
            gameOver = false;
            score = 0;
            lives = 3;
            playerKmh = PLAYER_INITIAL_KMH;
            playerBaseSpeed = KMH_TO_PIXELS_PER_FRAME(PLAYER_INITIAL_KMH);
            playerMaxSpeed = playerBaseSpeed;
            activeOppositeCars = 0;
            initOppositeCars();
            initLaneDividers();
            // Re-enable the window close (X) button
            HWND hwnd = FindWindowA(NULL, "Lane Dodge Car Game");
            if (hwnd) {
                HMENU hMenu = GetSystemMenu(hwnd, FALSE);
                if (hMenu) {
                    EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
                }
            }
            // Restart the timer so the game resumes
            glutTimerFunc(1000 / 60, timer, 0);
        }
        glutPostRedisplay();
        return;
    }
    // Check Quit button
    if (x >= quitBtnX && x <= quitBtnX + btnW && oglY >= quitBtnY && oglY <= quitBtnY + btnH) {
        exit(0);
    }
}
#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <string>
#include<iostream>
#include "constants.h"
#include "utils.h"
#include "player.h"
#include "vehicle.h"
#include "road.h"
#include "hud.h"
#include "input.h"
using namespace std;




// Get lane position x with offset
// Structures





float lanePosX(int lane, int offset, float carWidth) {
    float base = ROAD_LEFT + lane * LANE_WIDTH;
    float step = (LANE_WIDTH - carWidth) / 3.0f;
    return base + offset * step;
}

// Global variable definitions



int checkCollision() {
    float playerX = lanePosX(playerLane, playerLaneOffset);
    float playerTop = playerY + CAR_HEIGHT;
    float playerBottom = playerY;
    for (int i = 0; i < NUM_OPPOSITE_CARS; i++) {
        if (!oppositeCars[i].active) continue; // Skip inactive (damaged) vehicles
        float oppX = lanePosX(oppositeCars[i].lane, 2, oppositeCars[i].width);
        float oppY = oppositeCars[i].y;
        float oppBottom = oppY;
        float oppTop = oppY + oppositeCars[i].height;
        if (playerTop > oppBottom && playerBottom < oppBottom &&
            oppX < playerX + CAR_WIDTH && oppX + oppositeCars[i].width > playerX) {
            return i;
        }
    }
    return -1;
}

// Draw crash effect (simple red/yellow starburst)
void drawCrashEffect() {
    glPushMatrix();
    glTranslatef(crashX + CAR_WIDTH/2, crashY + CAR_HEIGHT/2, 0);
    glColor3f(1, 0.2f, 0.1f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 12; i++) {
        float angle = i * 2 * 3.14159f / 12;
        float r = (i % 2 == 0) ? 40 : 20;
        glVertex2f(cos(angle) * r, sin(angle) * r);
    }
    glEnd();
    glColor3f(1, 1, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 12; i++) {
        float angle = i * 2 * 3.14159f / 12 + 0.13f;
        float r = (i % 2 == 0) ? 22 : 10;
        glVertex2f(cos(angle) * r, sin(angle) * r);
    }
    glEnd();
    glPopMatrix();
}



// Draws the Play/Quit menu (for both startup and game over)
void drawMenu(bool isGameOver) {
    char buf[128];
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float boxW = 420, boxH = isGameOver ? 260 : 200;
    float boxX = SCREEN_WIDTH/2 - boxW/2, boxY = SCREEN_HEIGHT/2 - boxH/2;
    // Background box
    glColor4f(0.08f, 0.08f, 0.08f, 0.92f);
    glBegin(GL_QUADS);
    glVertex2f(boxX, boxY);
    glVertex2f(boxX + boxW, boxY);
    glVertex2f(boxX + boxW, boxY + boxH);
    glVertex2f(boxX, boxY + boxH);
    glEnd();
    // Border
    glLineWidth(4);
    glColor3f(0.2f, 0.8f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boxX, boxY);
    glVertex2f(boxX + boxW, boxY);
    glVertex2f(boxX + boxW, boxY + boxH);
    glVertex2f(boxX, boxY + boxH);
    glEnd();
    glLineWidth(1);

    // Title
    if (isGameOver) {
        sprintf(buf, "GAME OVER");
        glColor3f(1, 0.2f, 0.2f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 80);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        // Final score
        sprintf(buf, "Final Score: %d", score);
        glColor3f(1, 1, 0.2f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 40);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        // High score
        sprintf(buf, "High Score: %d", highScore);
        glColor3f(0.5f, 1.0f, 0.5f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 + 10);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    } else {
        sprintf(buf, "LANE DODGE CAR GAME");
        glColor3f(0.2f, 1.0f, 0.7f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT / 2 + 60);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        sprintf(buf, "by Bikash0717");
        glColor3f(0.7f, 0.7f, 1.0f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 + 30);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Draw Play button
    float btnW = 120, btnH = 40;
    float playBtnX = SCREEN_WIDTH/2 - btnW - 20, playBtnY = SCREEN_HEIGHT/2 - 40;
    float quitBtnX = SCREEN_WIDTH/2 + 20, quitBtnY = playBtnY;
    // Play button
    glColor3f(0.2f, 0.8f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(playBtnX, playBtnY);
    glVertex2f(playBtnX + btnW, playBtnY);
    glVertex2f(playBtnX + btnW, playBtnY + btnH);
    glVertex2f(playBtnX, playBtnY + btnH);
    glEnd();
    glColor3f(0, 0.2f, 0);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(playBtnX, playBtnY);
    glVertex2f(playBtnX + btnW, playBtnY);
    glVertex2f(playBtnX + btnW, playBtnY + btnH);
    glVertex2f(playBtnX, playBtnY + btnH);
    glEnd();
    glLineWidth(1);
    sprintf(buf, "PLAY");
    glColor3f(1, 1, 1);
    glRasterPos2f(playBtnX + 32, playBtnY + 25);
    for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    // Quit button
    glColor3f(0.8f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(quitBtnX, quitBtnY);
    glVertex2f(quitBtnX + btnW, quitBtnY);
    glVertex2f(quitBtnX + btnW, quitBtnY + btnH);
    glVertex2f(quitBtnX, quitBtnY + btnH);
    glEnd();
    glColor3f(0.2f, 0, 0);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex2f(quitBtnX, quitBtnY);
    glVertex2f(quitBtnX + btnW, quitBtnY);
    glVertex2f(quitBtnX + btnW, quitBtnY + btnH);
    glVertex2f(quitBtnX, quitBtnY + btnH);
    glEnd();
    glLineWidth(1);
    sprintf(buf, "QUIT");
    glColor3f(1, 1, 1);
    glRasterPos2f(quitBtnX + 32, quitBtnY + 25);
    for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    // Instructions
    glColor3f(0.8f, 0.8f, 1.0f);
    if (isGameOver) {
        sprintf(buf, "Click PLAY to Restart or QUIT to Exit");
        glRasterPos2f(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 70);
    } else {
        sprintf(buf, "Click PLAY to Start or QUIT to Exit");
        glRasterPos2f(SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 - 70);
    }
    for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    glDisable(GL_BLEND);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    extern bool showMenu;
    if (showMenu) {
        drawMenu(false);
        glutSwapBuffers();
        return;
    }
    if (gameOver) {
        drawMenu(true);
        glutSwapBuffers();
        return;
    }
    drawRoadSides();
    drawLaneDividers();
    if (playerCrashed && respawnTimer > 0) {
        drawCrashEffect();
    }
    drawPlayerCar();
    drawOppositeCars();
    // Do NOT draw the crashed vehicle (damaged/removed)
    drawHUD();
    glutSwapBuffers();
}

void timer(int value) {
    // Remove playerY movement so player car stays fixed

    // Move lane dividers down always at playerBaseSpeed (simulate road movement)
    for (int i = 0; i < NUM_LANE_DIVIDERS; i++) {
        laneDividers[i].y -= playerBaseSpeed;
        if (laneDividers[i].y < -40) laneDividers[i].y = SCREEN_HEIGHT;
    }

    // Handle audio transitions (call from game.cpp)
    extern void handleAudio();
    handleAudio();

    // Gradually increase traffic and player speed
    trafficIncreaseTimer++;
    int initialDelay = 120; // 2 seconds at 60 FPS
    int spawnInterval = 480; // 8 seconds at 60 FPS
    static int playerSpeedIncreaseTimer = 0;
    playerSpeedIncreaseTimer++;
    if (activeOppositeCars < NUM_OPPOSITE_CARS) {
        if ((activeOppositeCars == 0 && trafficIncreaseTimer >= initialDelay) ||
            (activeOppositeCars > 0 && trafficIncreaseTimer >= spawnInterval)) {
            activeOppositeCars++;
            trafficIncreaseTimer = 0;
        }
    }
    // Increase player speed by 10 km/hr every 10 seconds
    if (playerSpeedIncreaseTimer >= 600) {
        playerSpeedIncreaseTimer = 0;
        playerKmh += PLAYER_SPEEDUP_KMH;
        playerBaseSpeed = KMH_TO_PIXELS_PER_FRAME(playerKmh);
        playerMaxSpeed = playerBaseSpeed;
        // Opposing car speeds remain fixed
    }

    // If player is crashed, handle respawn timer
    if (playerCrashed) {
        respawnTimer--;
        if (respawnTimer <= 0) {
            playerCrashed = false;
            playerVisible = true;
        } else if (respawnTimer < RESPAWN_DELAY/2) {
            playerVisible = true; // Fade in
        } else {
            playerVisible = false;
        }
        glutPostRedisplay();
        glutTimerFunc(1000 / 60, timer, 0);
        return;
    }

    if (gameOver) {
        // Game Over: just count down and exit after delay
        gameOverTimer--;
        if (gameOverTimer <= 0) {
            exit(0);
        }
        glutPostRedisplay();
        glutTimerFunc(1000 / 60, timer, 0);
        return;
    }

    // Move only active cars (opposing cars always move at their own speed)
        for (int i = 0; i < activeOppositeCars; i++) {
        if (!oppositeCars[i].active) continue;

        // Smooth lane change logic
        if (oppositeCars[i].isChangingLane) {
                        oppositeCars[i].laneChangeProgress += 0.02f; // Speed of lane change
            if (oppositeCars[i].laneChangeProgress >= 1.0f) {
                oppositeCars[i].laneChangeProgress = 1.0f;
                oppositeCars[i].lane = oppositeCars[i].targetLane;
                oppositeCars[i].isChangingLane = false;
            }
            float startX = lanePosX(oppositeCars[i].lane, 2, oppositeCars[i].width);
            float endX = lanePosX(oppositeCars[i].targetLane, 2, oppositeCars[i].width);
            // Ease in-out interpolation for smoother effect
            float t = oppositeCars[i].laneChangeProgress;
            float easedT = t * t * (3.0f - 2.0f * t);
            oppositeCars[i].x = startX + (endX - startX) * easedT;
        } else {
            // Keep the car in its lane if not changing
            oppositeCars[i].x = lanePosX(oppositeCars[i].lane, 2, oppositeCars[i].width);
        }
        // Only the true rear-most BLOCKED vehicle in a lane can change lane if blocked
        // For vehicles moving downward, rear-most is the one with the highest y (closest to bottom)
        // First, check if this vehicle is blocked
        int carAheadIdx = -1;
        float minDist = 1e9;
        for (int j = 0; j < NUM_OPPOSITE_CARS; j++) {
            if (j == i) continue;
            if (!oppositeCars[j].active) continue;
            if (oppositeCars[j].lane == oppositeCars[i].lane &&
                oppositeCars[j].direction == -1 && oppositeCars[j].y < oppositeCars[i].y) {
                float dist = oppositeCars[i].y - (oppositeCars[j].y + oppositeCars[j].height);
                if (dist >= 0 && dist < minDist) {
                    minDist = dist;
                    carAheadIdx = j;
                }
            }
        }
        bool blocked = false;
        if (carAheadIdx != -1 && minDist < CAR_HEIGHT + 10) {
            blocked = true;
        }

        // Find the rear-most (highest y) among all blocked vehicles in this lane
        bool isRearMostBlocked = true;
        if (blocked) {
            for (int j = 0; j < NUM_OPPOSITE_CARS; j++) {
                if (j == i) continue;
                if (!oppositeCars[j].active) continue;
                if (oppositeCars[j].lane == oppositeCars[i].lane && oppositeCars[j].direction == -1) {
                    // Is j also blocked?
                    int jAheadIdx = -1;
                    float jMinDist = 1e9;
                    for (int k = 0; k < NUM_OPPOSITE_CARS; k++) {
                        if (k == j) continue;
                        if (!oppositeCars[k].active) continue;
                        if (oppositeCars[k].lane == oppositeCars[j].lane &&
                            oppositeCars[k].direction == -1 && oppositeCars[k].y < oppositeCars[j].y) {
                            float dist = oppositeCars[j].y - (oppositeCars[k].y + oppositeCars[k].height);
                            if (dist >= 0 && dist < jMinDist) {
                                jMinDist = dist;
                                jAheadIdx = k;
                            }
                        }
                    }
                    bool jBlocked = false;
                    if (jAheadIdx != -1 && jMinDist < CAR_HEIGHT + 10) {
                        jBlocked = true;
                    }
                    // If there is a blocked car with a higher y (closer to bottom), then this is not the rear-most
                    if (jBlocked && oppositeCars[j].y > oppositeCars[i].y) {
                        isRearMostBlocked = false;
                        break;
                    }
                }
            }
        }

        // --- New: Check if this car is about to hit the player from behind ---
        bool playerAhead = false;
        if (oppositeCars[i].lane == playerLane) {
            float oppFront = oppositeCars[i].y + oppositeCars[i].height;
            float playerBack = playerY;
            if (oppositeCars[i].y < playerY && oppFront > playerBack && playerY - (oppositeCars[i].y + oppositeCars[i].height) < CAR_HEIGHT + 10) {
                playerAhead = true;
            }
        }

        // Only set signalOn true if a lane change is performed in this frame
        oppositeCars[i].signalOn = false;

        if (((blocked && isRearMostBlocked) || playerAhead) && (blocked || playerAhead)) {
            bool changedLane = false;
            int leftLane = oppositeCars[i].lane - 1;
            int rightLane = oppositeCars[i].lane + 1;
            if (leftLane >= 0) {
                bool laneOccupied = false;
                for (int j = 0; j < NUM_OPPOSITE_CARS; j++) {
                    if (j != i && oppositeCars[j].active && oppositeCars[j].lane == leftLane &&
                        fabs(oppositeCars[j].y - oppositeCars[i].y) < CAR_HEIGHT + 10) {
                        laneOccupied = true;
                        break;
                    }
                }
                if (!laneOccupied && leftLane == playerLane && fabs(playerY - oppositeCars[i].y) < CAR_HEIGHT + 10) {
                    laneOccupied = true;
                }
                                if (!laneOccupied) {
                    if (!oppositeCars[i].isChangingLane) {
                        oppositeCars[i].isChangingLane = true;
                        oppositeCars[i].targetLane = leftLane;
                        oppositeCars[i].laneChangeProgress = 0.0f;
                    }
                    changedLane = true;
                    oppositeCars[i].signalOn = true;
                }
            }
            if (!changedLane && rightLane < NUM_LANES) {
                bool laneOccupied = false;
                for (int j = 0; j < NUM_OPPOSITE_CARS; j++) {
                    if (j != i && oppositeCars[j].active && oppositeCars[j].lane == rightLane &&
                        fabs(oppositeCars[j].y - oppositeCars[i].y) < CAR_HEIGHT + 10) {
                        laneOccupied = true;
                        break;
                    }
                }
                if (!laneOccupied && rightLane == playerLane && fabs(playerY - oppositeCars[i].y) < CAR_HEIGHT + 10) {
                    laneOccupied = true;
                }
                                if (!laneOccupied) {
                    if (!oppositeCars[i].isChangingLane) {
                        oppositeCars[i].isChangingLane = true;
                        oppositeCars[i].targetLane = rightLane;
                        oppositeCars[i].laneChangeProgress = 0.0f;
                    }
                    changedLane = true;
                    oppositeCars[i].signalOn = true;
                }
            }
            if (!changedLane) {
                float slowed = oppositeCars[i].lastSpeed * 0.4f;
                if (slowed < 0.01f) slowed = 0.01f;
                oppositeCars[i].speed = slowed;
            }
        } else {
            float restore = oppositeCars[i].lastSpeed;
            if (restore < 0.01f) restore = 0.01f;
            if (oppositeCars[i].speed != restore) {
                oppositeCars[i].speed = restore;
            }
        }
        if (oppositeCars[i].speed > 0.0f)
            oppositeCars[i].y += oppositeCars[i].direction * oppositeCars[i].speed;
        if (oppositeCars[i].y < -oppositeCars[i].height || !oppositeCars[i].active) {
            int newLane, attempts = 0;
            float newY;
            int newDir = -1; // Always downward
            bool overlapRespawn;
            do {
                overlapRespawn = false;
                newLane = rand() % NUM_LANES;
                newY = SCREEN_HEIGHT + (rand() % 200);
                for (int j = 0; j < NUM_OPPOSITE_CARS; j++) {
                    if (j != i && oppositeCars[j].active && oppositeCars[j].lane == newLane &&
                        oppositeCars[j].direction == newDir &&
                        fabs(oppositeCars[j].y - newY) < oppositeCars[i].height + 80) {
                        overlapRespawn = true;
                        break;
                    }
                }
                attempts++;
            } while (overlapRespawn && attempts < 20);
                        oppositeCars[i].lane = newLane;
            oppositeCars[i].x = lanePosX(newLane, 2, oppositeCars[i].width);
            oppositeCars[i].isChangingLane = false;
            oppositeCars[i].laneChangeProgress = 0.0f;
            oppositeCars[i].y = newY;
            oppositeCars[i].direction = newDir;
            setVehicleType(oppositeCars[i]);
            oppositeCars[i].active = true;
            oppositeCars[i].signalOn = false; // Ensure indicator is off after respawn
            score++;
        }
    }

    // Player car stays visually fixed

    // Check collision (only if not crashed)
    if (!playerCrashed) {
        int colIdx = checkCollision();
        if (colIdx != -1) {
            lives--;
            if (lives <= 0) {
                gameOver = true;
                gameOverTimer = GAME_OVER_DELAY;
                updateHighScore();
                // Disable the close button (X) when game is over
                HWND hwnd = FindWindowA(NULL, "Lane Dodge Car Game");
                if (hwnd) {
                    HMENU hMenu = GetSystemMenu(hwnd, FALSE);
                    if (hMenu) {
                        EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
                    }
                }
                glutPostRedisplay();
                return;
            } else {
                // Store crash position for effect
                crashX = lanePosX(playerLane, playerLaneOffset);
                crashY = playerY;
                playerCrashed = true;
                respawnTimer = RESPAWN_DELAY;
                playerVisible = false;
                crashedVehicleIdx = colIdx;
                oppositeCars[colIdx].active = false; // Hide the crashed vehicle
            }
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, timer, 0);
}
