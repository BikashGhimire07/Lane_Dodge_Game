#include <GL/glut.h>
#include "road.h"
#include "utils.h"
#include "game.h"
void updateLaneDividers() {
    for (int i = 0; i < NUM_LANE_DIVIDERS; i++) {
        laneDividers[i].y -= playerBaseSpeed;
        if (laneDividers[i].y < -40) {
            laneDividers[i].y = SCREEN_HEIGHT;
        }
    }
}





LaneDivider laneDividers[NUM_LANE_DIVIDERS];

void initLaneDividers() {
    for (int i = 0; i < NUM_LANE_DIVIDERS; i++) {
        laneDividers[i].x = SCREEN_WIDTH / 2;
        laneDividers[i].y = i * 70;
    }
}

void drawRoadSides() {
    glColor3f(0.2f, 0.7f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(ROAD_LEFT, 0);
    glVertex2f(ROAD_LEFT, SCREEN_HEIGHT); glVertex2f(0, SCREEN_HEIGHT);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(ROAD_RIGHT, 0); glVertex2f(SCREEN_WIDTH, 0);
    glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT); glVertex2f(ROAD_RIGHT, SCREEN_HEIGHT);
    glEnd();
    glColor3f(0.18f, 0.18f, 0.18f);
    glBegin(GL_QUADS);
    glVertex2f(ROAD_LEFT, 0);
    glVertex2f(ROAD_RIGHT, 0);
    glVertex2f(ROAD_RIGHT, SCREEN_HEIGHT);
    glVertex2f(ROAD_LEFT, SCREEN_HEIGHT);
    glEnd();
}

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
