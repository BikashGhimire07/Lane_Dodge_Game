#define GLUT_DISABLE_ATEXIT_HACK

#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

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
#define CAR_KMH 50.0f
#define BUS_KMH 35.0f
#define TRUCK_KMH 40.0f

// Conversion: 1 km/hr = 1000/3600 = 0.27778 m/s
// We'll map 1 km/hr to 0.18 pixels/frame at 60 FPS for a good feel
#define KMH_TO_PIXELS_PER_FRAME(kmh) ((kmh) * 0.18f)

float playerKmh = PLAYER_INITIAL_KMH;
float playerBaseSpeed = KMH_TO_PIXELS_PER_FRAME(PLAYER_INITIAL_KMH);
float playerMinSpeed = 0.0f;
float playerMaxSpeed = KMH_TO_PIXELS_PER_FRAME(PLAYER_INITIAL_KMH);

// Structures
enum VehicleType { VEHICLE_CAR, VEHICLE_BUS, VEHICLE_TRUCK };

struct Car {
    int lane;
    float y;
    float r, g, b;
    int direction; // 1 for upward (same as player)
    float speed; // Individual speed for each car
    float lastSpeed; // Store the assigned speed for restoration
    VehicleType type;
    float width, height;
    bool active; // If false, car is not drawn or moved (damaged/removed)
    bool signalOn; // If true, show turn indicator (for lane change)
} oppositeCars[NUM_OPPOSITE_CARS];

struct LaneDivider {
    float x, y;
} laneDividers[NUM_LANE_DIVIDERS];

// Player variables
int playerLane = 1;
int playerLaneOffset = 2;
float playerY = PLAYER_START_Y;
int score = 0;
int lives = 3;
int activeOppositeCars = 0; // Start with 0 cars, only player at first
int trafficIncreaseTimer = 0; // Timer for increasing traffic
// Player always moves at playerBaseSpeed

// Game over state
bool gameOver = false;
int gameOverTimer = 0; // frames to show Game Over screen before exit
const int GAME_OVER_DELAY = 120; // 2 seconds at 60 FPS

// Crash/respawn state
bool playerCrashed = false;
float crashX = 0, crashY = 0;
int respawnTimer = 0; // frames remaining until respawn
const int RESPAWN_DELAY = 180; // 3 seconds at 60 FPS
bool playerVisible = true;
int crashedVehicleIdx = -1; // Index of the vehicle the player crashed into

// Get lane position x with offset
float lanePosX(int lane, int offset, float carWidth = CAR_WIDTH) {
    float base = ROAD_LEFT + lane * LANE_WIDTH;
    float step = (LANE_WIDTH - carWidth) / 3.0f;
    return base + offset * step;
}

// Initialize lane dividers
void initLaneDividers() {
    for (int i = 0; i < NUM_LANE_DIVIDERS; i++) {
        laneDividers[i].x = SCREEN_WIDTH / 2;
        laneDividers[i].y = i * 70;
    }
}

// Vehicle size, color, and speed presets
void setVehicleType(Car& c) {
    int t = rand() % 3;
    if (t == 0) { // Car
        c.type = VEHICLE_CAR;
        c.width = 50;
        c.height = 70;
        c.r = 0.95f; c.g = 0.15f; c.b = 0.15f; // Classy red
    } else if (t == 1) { // Bus
        c.type = VEHICLE_BUS;
        c.width = 60;
        c.height = 110;
        c.r = 0.1f; c.g = 0.5f; c.b = 0.85f; // Blue bus
    } else { // Truck
        c.type = VEHICLE_TRUCK;
        c.width = 70;
        c.height = 90;
        c.r = 0.8f; c.g = 0.7f; c.b = 0.2f; // Gold truck
    }
    // Set fixed real-world speed for each vehicle type (in km/hr, converted to pixels/frame)
    float baseSpeed;
    if (c.type == VEHICLE_CAR) {
        baseSpeed = KMH_TO_PIXELS_PER_FRAME(CAR_KMH);
    } else if (c.type == VEHICLE_BUS) {
        baseSpeed = KMH_TO_PIXELS_PER_FRAME(BUS_KMH);
    } else { // Truck
        baseSpeed = KMH_TO_PIXELS_PER_FRAME(TRUCK_KMH);
    }
    c.speed = baseSpeed;
    c.lastSpeed = baseSpeed;
    c.active = true;
    c.signalOn = false;
}

// Update only the speed of a vehicle based on its type (do not change type or appearance)
void updateVehicleSpeedByType(Car& c) {
    float baseSpeed;
    if (c.type == VEHICLE_CAR) {
        baseSpeed = KMH_TO_PIXELS_PER_FRAME(CAR_KMH);
    } else if (c.type == VEHICLE_BUS) {
        baseSpeed = KMH_TO_PIXELS_PER_FRAME(BUS_KMH);
    } else { // Truck
        baseSpeed = KMH_TO_PIXELS_PER_FRAME(TRUCK_KMH);
    }
    c.speed = baseSpeed;
    c.lastSpeed = baseSpeed;
}

// Initialize opposite cars
void initOppositeCars() {
    srand(time(NULL));
    for (int i = 0; i < NUM_OPPOSITE_CARS; i++) {
        oppositeCars[i].lane = rand() % NUM_LANES;
        // Spawn at the top of the screen
        oppositeCars[i].y = SCREEN_HEIGHT + (rand() % 200);
        setVehicleType(oppositeCars[i]);
        oppositeCars[i].direction = -1; // Move downward
    }
}

// Draw road and sides
void drawRoadSides() {
    // Grass
    glColor3f(0.2f, 0.7f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(ROAD_LEFT, 0);
    glVertex2f(ROAD_LEFT, SCREEN_HEIGHT); glVertex2f(0, SCREEN_HEIGHT);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(ROAD_RIGHT, 0); glVertex2f(SCREEN_WIDTH, 0);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT); glVertex2f(ROAD_RIGHT, SCREEN_HEIGHT);
    glEnd();
    // Road
    glColor3f(0.18f, 0.18f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(ROAD_LEFT, 0);
    glVertex2f(ROAD_RIGHT, 0);
    glVertex2f(ROAD_RIGHT, SCREEN_HEIGHT);
    glVertex2f(ROAD_LEFT, SCREEN_HEIGHT);
    glEnd();
}

// Draw a car, bus, or truck (classy look)
void drawVehicle(const Car& c) {
    float w = c.width, h = c.height;
    // Body
    glColor3f(c.r, c.g, c.b);
    glBegin(GL_POLYGON);
    glVertex2f(6, 12); glVertex2f(w-6, 12); glVertex2f(w-2, h-18); glVertex2f(2, h-18);
    glEnd();
    // Roof
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(12, h-30); glVertex2f(w-12, h-30); glVertex2f(w-10, h-12); glVertex2f(10, h-12);
    glEnd();
    // Windows
    glColor3f(0.7f, 0.9f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(16, h-28); glVertex2f(w-16, h-28); glVertex2f(w-14, h-18); glVertex2f(14, h-18);
    glEnd();
    // Wheels
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(8, 8); glVertex2f(18, 8); glVertex2f(18, 16); glVertex2f(8, 16);
    glVertex2f(w-18, 8); glVertex2f(w-8, 8); glVertex2f(w-8, 16); glVertex2f(w-18, 16);
    glVertex2f(8, h-16); glVertex2f(18, h-16); glVertex2f(18, h-8); glVertex2f(8, h-8);
    glVertex2f(w-18, h-16); glVertex2f(w-8, h-16); glVertex2f(w-8, h-8); glVertex2f(w-18, h-8);
    glEnd();
    // Headlights/taillights
    if (c.type == VEHICLE_CAR) {
        glColor3f(1.0f, 1.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(w/2-8, h-12); glVertex2f(w/2+8, h-12); glVertex2f(w/2+8, h-6); glVertex2f(w/2-8, h-6);
        glEnd();
    } else if (c.type == VEHICLE_BUS) {
        glColor3f(1.0f, 0.9f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(w/2-20, h-12); glVertex2f(w/2+20, h-12); glVertex2f(w/2+20, h-6); glVertex2f(w/2-20, h-6);
        glEnd();
    } else if (c.type == VEHICLE_TRUCK) {
        glColor3f(0.1f, 0.1f, 0.1f); // Black, like wheels
        glBegin(GL_QUADS);
        glVertex2f(w-16, h-18); glVertex2f(w-6, h-18); glVertex2f(w-6, h-10); glVertex2f(w-16, h-10);
        glEnd();
    }
    // Turn signal indicator (drawn if c.active && c.signalOn)
    if (c.active && c.signalOn) {
        glColor3f(1.0f, 1.0f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(w/2-8, 0); glVertex2f(w/2+8, 0); glVertex2f(w/2+8, 8); glVertex2f(w/2-8, 8);
        glEnd();
    }
}

// Draw player car, with optional fade-in
void drawPlayerCar() {
    if (!playerVisible) return;
    float playerX = lanePosX(playerLane, playerLaneOffset);
    float alpha = 1.0f;
    if (playerCrashed && respawnTimer > 0 && respawnTimer < RESPAWN_DELAY/2) {
        // Fade in during last half of respawn
        alpha = 1.0f - (float)respawnTimer / (RESPAWN_DELAY/2);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, alpha);
    glBegin(GL_POLYGON);
    glVertex2f(playerX + 8, playerY + 10);
    glVertex2f(playerX + CAR_WIDTH - 8, playerY + 10);
    glVertex2f(playerX + CAR_WIDTH - 4, playerY + 20);
    glVertex2f(playerX + CAR_WIDTH - 2, playerY + 30);
    glVertex2f(playerX + CAR_WIDTH - 4, playerY + CAR_HEIGHT - 12);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + CAR_HEIGHT - 4);
    glVertex2f(playerX + 10, playerY + CAR_HEIGHT - 4);
    glVertex2f(playerX + 4, playerY + CAR_HEIGHT - 12);
    glVertex2f(playerX + 2, playerY + 30);
    glVertex2f(playerX + 4, playerY + 20);
    glEnd();
    glColor4f(0.1f, 0.3f, 0.8f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH / 2 - 4, playerY + 10);
    glVertex2f(playerX + CAR_WIDTH / 2 + 4, playerY + 10);
    glVertex2f(playerX + CAR_WIDTH / 2 + 8, playerY + CAR_HEIGHT - 4);
    glVertex2f(playerX + CAR_WIDTH / 2 - 8, playerY + CAR_HEIGHT - 4);
    glEnd();
    glColor4f(0.85f, 0.85f, 0.85f, alpha);
    glBegin(GL_POLYGON);
    glVertex2f(playerX + 12, playerY + CAR_HEIGHT - 32);
    glVertex2f(playerX + CAR_WIDTH - 12, playerY + CAR_HEIGHT - 32);
    glVertex2f(playerX + CAR_WIDTH - 8, playerY + CAR_HEIGHT - 18);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + CAR_HEIGHT - 10);
    glVertex2f(playerX + 10, playerY + CAR_HEIGHT - 10);
    glVertex2f(playerX + 8, playerY + CAR_HEIGHT - 18);
    glEnd();
    glColor4f(0.5f, 0.8f, 1.0f, alpha);
    glBegin(GL_POLYGON);
    glVertex2f(playerX + 16, playerY + CAR_HEIGHT - 28);
    glVertex2f(playerX + CAR_WIDTH - 16, playerY + CAR_HEIGHT - 28);
    glVertex2f(playerX + CAR_WIDTH - 14, playerY + CAR_HEIGHT - 18);
    glVertex2f(playerX + 14, playerY + CAR_HEIGHT - 18);
    glEnd();
    glColor4f(0.1f, 0.1f, 0.1f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(playerX + 16, playerY + 6);
    glVertex2f(playerX + 19, playerY + 6);
    glVertex2f(playerX + 19, playerY + 14);
    glVertex2f(playerX + 16, playerY + 14);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH - 19, playerY + 6);
    glVertex2f(playerX + CAR_WIDTH - 16, playerY + 6);
    glVertex2f(playerX + CAR_WIDTH - 16, playerY + 14);
    glVertex2f(playerX + CAR_WIDTH - 19, playerY + 14);
    glEnd();
    glColor4f(0.7f, 0.8f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(playerX + 10, playerY + 2);
    glVertex2f(playerX + 15, playerY + 2);
    glVertex2f(playerX + 15, playerY + 8);
    glVertex2f(playerX + 10, playerY + 8);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH - 15, playerY + 2);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + 2);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + 8);
    glVertex2f(playerX + CAR_WIDTH - 15, playerY + 8);
    glEnd();
    glColor4f(0.2f, 0.4f, 1.0f, alpha);
    glBegin(GL_LINE_LOOP);
    glVertex2f(playerX + 9, playerY + 1);
    glVertex2f(playerX + 16, playerY + 1);
    glVertex2f(playerX + 16, playerY + 9);
    glVertex2f(playerX + 9, playerY + 9);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2f(playerX + CAR_WIDTH - 16, playerY + 1);
    glVertex2f(playerX + CAR_WIDTH - 9, playerY + 1);
    glVertex2f(playerX + CAR_WIDTH - 9, playerY + 9);
    glVertex2f(playerX + CAR_WIDTH - 16, playerY + 9);
    glEnd();
    glColor4f(0, 0, 0, alpha);
    glBegin(GL_QUADS);
    glVertex2f(playerX + 4, playerY + 8);
    glVertex2f(playerX + 10, playerY + 8);
    glVertex2f(playerX + 10, playerY + 18);
    glVertex2f(playerX + 4, playerY + 18);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + 8);
    glVertex2f(playerX + CAR_WIDTH - 4, playerY + 8);
    glVertex2f(playerX + CAR_WIDTH - 4, playerY + 18);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + 18);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + 4, playerY + CAR_HEIGHT - 18);
    glVertex2f(playerX + 10, playerY + CAR_HEIGHT - 18);
    glVertex2f(playerX + 10, playerY + CAR_HEIGHT - 8);
    glVertex2f(playerX + 4, playerY + CAR_HEIGHT - 8);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + CAR_HEIGHT - 18);
    glVertex2f(playerX + CAR_WIDTH - 4, playerY + CAR_HEIGHT - 18);
    glVertex2f(playerX + CAR_WIDTH - 4, playerY + CAR_HEIGHT - 8);
    glVertex2f(playerX + CAR_WIDTH - 10, playerY + CAR_HEIGHT - 8);
    glEnd();
    glColor4f(0.8f, 0.8f, 0.8f, alpha);
    glBegin(GL_QUADS);
    glVertex2f(playerX + 6, playerY + 11);
    glVertex2f(playerX + 8, playerY + 11);
    glVertex2f(playerX + 8, playerY + 15);
    glVertex2f(playerX + 6, playerY + 15);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH - 8, playerY + 11);
    glVertex2f(playerX + CAR_WIDTH - 6, playerY + 11);
    glVertex2f(playerX + CAR_WIDTH - 6, playerY + 15);
    glVertex2f(playerX + CAR_WIDTH - 8, playerY + 15);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + 6, playerY + CAR_HEIGHT - 15);
    glVertex2f(playerX + 8, playerY + CAR_HEIGHT - 15);
    glVertex2f(playerX + 8, playerY + CAR_HEIGHT - 11);
    glVertex2f(playerX + 6, playerY + CAR_HEIGHT - 11);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(playerX + CAR_WIDTH - 8, playerY + CAR_HEIGHT - 15);
    glVertex2f(playerX + CAR_WIDTH - 6, playerY + CAR_HEIGHT - 15);
    glVertex2f(playerX + CAR_WIDTH - 6, playerY + CAR_HEIGHT - 11);
    glVertex2f(playerX + CAR_WIDTH - 8, playerY + CAR_HEIGHT - 11);
    glEnd();
    glDisable(GL_BLEND);
}

// Draw opposite cars
void drawOppositeCars() {
    for (int i = 0; i < NUM_OPPOSITE_CARS; i++) {
        if (!oppositeCars[i].active) continue;
        float x = lanePosX(oppositeCars[i].lane, 2, oppositeCars[i].width);
        float y = oppositeCars[i].y;
        glPushMatrix();
        glTranslatef(x, y, 0);
        drawVehicle(oppositeCars[i]);
        glPopMatrix();
    }
}

// Draw lane dividers
void drawLaneDividers() {
    glColor3f(1, 1, 1);
    for (int lane = 1; lane < NUM_LANES; lane++) {
        float x = ROAD_LEFT + lane * LANE_WIDTH;
        for (int i = 0; i < NUM_LANE_DIVIDERS; i++) {
            glBegin(GL_QUADS);
            glVertex2f(x - 3, laneDividers[i].y);
            glVertex2f(x + 3, laneDividers[i].y);
            glVertex2f(x + 3, laneDividers[i].y + 40);
            glVertex2f(x - 3, laneDividers[i].y + 40);
            glEnd();
        }
    }
}

// Draw HUD
void drawHUD() {
    char buf[64];
    glColor3f(1, 1, 0);
    sprintf(buf, "Score: %d", score);
    glRasterPos2f(10, SCREEN_HEIGHT - 20);
    for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    glColor3f(1, 0.2f, 0.2f);
    sprintf(buf, "Lives: %d", lives);
    glRasterPos2f(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 20);
    for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

// Check collision
// Returns index of collided vehicle, or -1 if none
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

// Display callback
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
        // Draw score below with shadow
        sprintf(buf, "Final Score: %d", score);
        glColor3f(0, 0, 0);
        glRasterPos2f(SCREEN_WIDTH / 2 - 82, SCREEN_HEIGHT / 2 - 2);
        for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glColor3f(1, 1, 0.2f);
        glRasterPos2f(SCREEN_WIDTH / 2 - 90, SCREEN_HEIGHT / 2);
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

// Timer callback
void timer(int value) {
    // Remove playerY movement so player car stays fixed

    // Move lane dividers down always at playerBaseSpeed (simulate road movement)
    for (int i = 0; i < NUM_LANE_DIVIDERS; i++) {
        laneDividers[i].y -= playerBaseSpeed;
        if (laneDividers[i].y < -40) laneDividers[i].y = SCREEN_HEIGHT;
    }

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
                    oppositeCars[i].lane = leftLane;
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
                    oppositeCars[i].lane = rightLane;
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
                glutPostRedisplay();
                glutTimerFunc(1000 / 60, timer, 0);
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

// Keyboard callback
void keyboard(int key, int x, int y) {
    if (playerCrashed) return; // Ignore input while crashed
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
            else if (playerLane < NUM_LANES - 1) {
                playerLane++;
                playerLaneOffset = 0;
            }
            break;
        // Up/Down arrows no longer affect speed
    }
    glutPostRedisplay();
}

// Main function
int main(int argc, char** argv) {
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
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}