

// All relevant headers must be included at the top
#include "constants.h"
#include "utils.h"
#include "game.h"
#include "player.h"
#include "vehicle.h"
#include "road.h"
#include "hud.h"
#include "input.h"
#include "utils.h"
#include <GL/glut.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

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

/* 
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 800
#define CAR_WIDTH 50 // Increased so only one car fits per lane
#define CAR_HEIGHT 70
#define PLAYER_START_X (SCREEN_WIDTH / 2 - CAR_WIDTH / 2)
#define PLAYER_START_Y 50
#define NUM_OPPOSITE_CARS 6 // Maximum number of cars
#define NUM_LANE_DIVIDERS 15
#define LANE_WIDTH 120
#define ROAD_WIDTH (NUM_LANES * LANE_WIDTH)
#define ROAD_LEFT ((SCREEN_WIDTH - ROAD_WIDTH) / 2)
#define ROAD_RIGHT (ROAD_LEFT + ROAD_WIDTH)
#define NUM_LANES 4 // Changed to 4 lanes
// --- Real-world speed settings (km/hr) ---
#define PLAYER_INITIAL_KMH 60.0f
#define PLAYER_SPEEDUP_KMH 10.0f
#define CAR_KMH 40.0f
#define BUS_KMH 40.0f
#define TRUCK_KMH 40.0f */

// Conversion: 1 km/hr = 1000/3600 = 0.27778 m/s
// We'll map 1 km/hr to 0.18 pixels/frame at 60 FPS for a good feel
#define KMH_TO_PIXELS_PER_FRAME(kmh) ((kmh) * 0.18f)


float playerKmh = PLAYER_INITIAL_KMH;
float playerBaseSpeed = KMH_TO_PIXELS_PER_FRAME(PLAYER_INITIAL_KMH);
float playerMinSpeed = 0.0f;
float playerMaxSpeed = KMH_TO_PIXELS_PER_FRAME(PLAYER_INITIAL_KMH);
int playerLane = 1;
int playerLaneOffset = 2;
float playerY = PLAYER_START_Y;
int score = 0;
int highScore = 0;
int lives = 3;
int activeOppositeCars = 0;
int trafficIncreaseTimer = 0;
bool gameOver = false;
bool newHighScore = false;
int gameOverTimer = 0;
const int GAME_OVER_DELAY = 120;
bool playerCrashed = false;
float crashX = 0, crashY = 0;
int respawnTimer = 0;
const int RESPAWN_DELAY = 180;
bool playerVisible = true;
int crashedVehicleIdx = -1;

// Audio state
bool normalMusicPlaying = false;
bool crashMusicPlaying = false;

// Structures



// Forward declarations to ensure visibility
void drawRoadSides();
void drawLaneDividers();
void drawOppositeCars();
void drawPlayerCar();
void drawHUD();

// Forward declaration for audio/game state
void handleAudio();




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
    glutTimerFunc(0, timer, 0);

    // Start normal background music
    playMP3("normal", "audio/normal.mp3", true);
    normalMusicPlaying = true;
    crashMusicPlaying = false;

    glutMainLoop();
    return 0;
}

// Call this in your main game loop/timer to handle audio transitions
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