

// =================== Includes ===================
#include <cstdio>
#include <cstdlib>
#include <GL/glut.h>
#include "constants.h"
#include "utils.h"
#include "game.h"
#include "player.h"
#include "vehicle.h"
#include "road.h"
#include "hud.h"
#include "input.h"

// Audio state variables defined in utils.cpp
extern bool normalMusicPlaying;
extern bool crashMusicPlaying;

// =================== Global Variables ===================
int score = 0, highScore = 0, lives = 3, activeOppositeCars = 0, trafficIncreaseTimer = 0, gameOverTimer = 0, crashedVehicleIdx = -1, playerLane = 1, playerLaneOffset = 0, respawnTimer = 0;
float playerKmh = PLAYER_INITIAL_KMH, playerBaseSpeed = KMH_TO_PIXELS_PER_FRAME(PLAYER_INITIAL_KMH), playerMinSpeed = 0.0f, playerMaxSpeed = 200.0f, playerY = PLAYER_START_Y, crashX = 0.0f, crashY = 0.0f;
bool gameOver = false, newHighScore = false, playerCrashed = false, playerVisible = true;
const int GAME_OVER_DELAY = 120;
const int RESPAWN_DELAY = 60;
bool showMenu = true;

// =================== GLUT Mouse Constants ===================
#ifndef GLUT_LEFT_BUTTON
#define GLUT_LEFT_BUTTON 0
#endif
#ifndef GLUT_DOWN
#define GLUT_DOWN 0
#endif

// =================== Function Declarations ===================
void mouseMenu(int button, int state, int x, int y);
void playMP3(const char* alias, const char* filename, bool loop);
void stopMP3(const char* alias);
void handleAudio();
void drawRoadSides();
void drawLaneDividers();
void drawOppositeCars();
void drawPlayerCar();
void drawHUD();

// =================== Main Function ===================
int main(int argc, char** argv) {
    loadHighScore();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    int win = glutCreateWindow("Lane Dodge Car Game");
    if (win <= 0) {
        printf("Failed to create window!\n");
        return 1;
    }

    gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
    initLaneDividers();
    initOppositeCars();
    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutKeyboardFunc(keyboardUp);
    glutMouseFunc(mouseMenu); // Register mouse handler for menu
    glutTimerFunc(0, timer, 0);

    // Start normal background music
    playMP3("normal", "audio/normal.mp3", true);
    normalMusicPlaying = true;
    crashMusicPlaying = false;

    glutMainLoop();
    return 0;
}