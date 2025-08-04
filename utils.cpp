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



void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawRoadSides();
    drawLaneDividers();
    if (gameOver) {
        // Draw Game Over UI
        char buf[64];
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.08f, 0.08f, 0.08f, 0.85f);
        float boxW = 380, boxH = 140;
        float boxX = SCREEN_WIDTH/2 - boxW/2, boxY = SCREEN_HEIGHT/2 - boxH/2;
        glBegin(GL_QUADS);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxW, boxY);
        glVertex2f(boxX + boxW, boxY + boxH);
        glVertex2f(boxX, boxY + boxH);
        glEnd();
        // Border
        glLineWidth(4);
        glColor3f(1, 0.2f, 0.2f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(boxX, boxY);
        glVertex2f(boxX + boxW, boxY);
        glVertex2f(boxX + boxW, boxY + boxH);
        glVertex2f(boxX, boxY + boxH);
        glEnd();
        glLineWidth(1);
        // Draw shadow for 'GAME OVER'
        sprintf(buf, "GAME OVER");
        glColor3f(0, 0, 0);
        glRasterPos2f(SCREEN_WIDTH / 2 - 92, SCREEN_HEIGHT / 2 + 38);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        // Main text
        glColor3f(1, 0.2f, 0.2f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 40);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        // Draw score and high score with shadow
        sprintf(buf, "Final Score: %d", score);
        glColor3f(0, 0, 0);
        glRasterPos2f(SCREEN_WIDTH / 2 - 82, SCREEN_HEIGHT / 2 - 2);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glColor3f(1, 1, 0.2f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        
        // Draw high score
        sprintf(buf, "High Score: %d", highScore);
        glColor3f(0, 0, 0);
        glRasterPos2f(SCREEN_WIDTH / 2 - 82, SCREEN_HEIGHT / 2 - 32);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glColor3f(0.5f, 1.0f, 0.5f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2 - 30);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        
        // Draw restart/quit instructions
        glColor3f(0.8f, 0.8f, 1.0f);
        sprintf(buf, "Press 'R' to Restart   or   'Q' to Quit");
        glRasterPos2f(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 50);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        
        glDisable(GL_BLEND);
        glutSwapBuffers();
        return;
    }
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
