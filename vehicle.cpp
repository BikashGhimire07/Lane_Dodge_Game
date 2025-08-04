
// =================== Includes ===================
#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "vehicle.h"
#include "utils.h"
#include "game.h"

Car oppositeCars[NUM_OPPOSITE_CARS];

void setVehicleType(Car& c) {
    int t = rand() % 3;
    if (t == 0) {
        c.type = VEHICLE_CAR;
        c.width = 50;
        c.height = 70;
        c.r = 0.95f; c.g = 0.15f; c.b = 0.15f;
    } else if (t == 1) {
        c.type = VEHICLE_BUS;
        c.width = 60;
        c.height = 110;
        c.r = 0.1f; c.g = 0.5f; c.b = 0.85f;
    } else {
        c.type = VEHICLE_TRUCK;
        c.width = 70;
        c.height = 90;
        c.r = 0.8f; c.g = 0.7f; c.b = 0.2f;
    }
    float baseSpeed;
    if (c.type == VEHICLE_CAR) baseSpeed = 50 * 0.18f;
    else if (c.type == VEHICLE_BUS) baseSpeed = 35 * 0.18f;
    else baseSpeed = 40 * 0.18f;
    c.speed = baseSpeed;
    c.lastSpeed = baseSpeed;
    c.active = true;
    c.signalOn = false;
}

void updateVehicleSpeedByType(Car& c) {
    float baseSpeed;
    if (c.type == VEHICLE_CAR) baseSpeed = 50 * 0.18f;
    else if (c.type == VEHICLE_BUS) baseSpeed = 35 * 0.18f;
    else baseSpeed = 40 * 0.18f;
    c.speed = baseSpeed;
    c.lastSpeed = baseSpeed;
}

void initOppositeCars() {
    srand((unsigned)time(NULL));
    for (int i = 0; i < NUM_OPPOSITE_CARS; i++) {
        oppositeCars[i].lane = i % NUM_LANES;
        oppositeCars[i].x = lanePosX(oppositeCars[i].lane, 2);
        oppositeCars[i].isChangingLane = false;
        oppositeCars[i].laneChangeProgress = 0.0f;
        oppositeCars[i].targetLane = oppositeCars[i].lane;
        oppositeCars[i].y = SCREEN_HEIGHT + (rand() % 200);
        setVehicleType(oppositeCars[i]);
        oppositeCars[i].direction = -1;
    }
}

void drawVehicle(const Car& c) {
    float w = c.width, h = c.height;
    glColor3f(c.r, c.g, c.b);
    glBegin(GL_POLYGON);
    glVertex2f(6, 12); glVertex2f(w-6, 12); glVertex2f(w-2, h-18); glVertex2f(2, h-18);
    glEnd();
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(12, h-30); glVertex2f(w-12, h-30); glVertex2f(w-10, h-12); glVertex2f(10, h-12);
    glEnd();
    glColor3f(0.7f, 0.9f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(16, h-28); glVertex2f(w-16, h-28); glVertex2f(w-14, h-18); glVertex2f(14, h-18);
    glEnd();
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(8, 8); glVertex2f(18, 8); glVertex2f(18, 16); glVertex2f(8, 16);
    glVertex2f(w-18, 8); glVertex2f(w-8, 8); glVertex2f(w-8, 16); glVertex2f(w-18, 16);
    glVertex2f(8, h-16); glVertex2f(18, h-16); glVertex2f(18, h-8); glVertex2f(8, h-8);
    glVertex2f(w-18, h-16); glVertex2f(w-8, h-16); glVertex2f(w-8, h-8); glVertex2f(w-18, h-8);
    glEnd();
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
        glColor3f(0.1f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(w-16, h-18); glVertex2f(w-6, h-18); glVertex2f(w-6, h-10); glVertex2f(w-16, h-10);
        glEnd();
    }
    if (c.active && c.signalOn) {
        glColor3f(1.0f, 1.0f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(w/2-8, 0); glVertex2f(w/2+8, 0); glVertex2f(w/2+8, 8); glVertex2f(w/2-8, 8);
        glEnd();
    }
}

void drawOppositeCars() {
    for (int i = 0; i < NUM_OPPOSITE_CARS; i++) {
        if (!oppositeCars[i].active) continue;
        glPushMatrix();
        glTranslatef(oppositeCars[i].x, oppositeCars[i].y, 0);
        drawVehicle(oppositeCars[i]);
        glPopMatrix();
    }
}
