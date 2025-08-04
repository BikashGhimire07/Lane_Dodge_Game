
#include <GL/glut.h>
#include <windows.h>
#include <cstdlib>
#include "input.h"
#include "utils.h"
#include "game.h"
#include "player.h"
#include "vehicle.h"
#include "road.h"
#include "hud.h"

#define PLAYER_START_Y 50
#define PLAYER_INITIAL_KMH 60.0f
#define KMH_TO_PIXELS_PER_FRAME(kmh) ((kmh) * 0.18f)

void keyboard(int key, int x, int y) {
    if (playerCrashed) return;
    switch (key) {
        case GLUT_KEY_LEFT:
            if (playerLaneOffset > 0) playerLaneOffset--;
            else if (playerLane > 0) {
                playerLane--;
                playerLaneOffset = 3;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (playerLaneOffset < 3) playerLaneOffset++;
            else if (playerLane < 4 - 1) {
                playerLane++;
                playerLaneOffset = 0;
            }
            break;
    }
    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
    if (showMenu || gameOver) {
        return;
    }
    HWND hwnd;
    HMENU hMenu;
    switch (key) {
        case 'r':
        case 'R':
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
            hwnd = FindWindowA(NULL, "Lane Dodge Car Game");
            if (hwnd) {
                hMenu = GetSystemMenu(hwnd, FALSE);
                if (hMenu) {
                    EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
                }
            }
            initOppositeCars();
            initLaneDividers();
            glutPostRedisplay();
            glutTimerFunc(1000 / 60, timer, 0);
            break;
        case 'q':
        case 'Q':
            exit(0);
            break;
    }
    glutPostRedisplay();
}
