


#include "hud.h"
#include "utils.h"
#include "game.h"
#include <GL/glut.h>
#include <fstream>
#include <cstdio>


void loadHighScore() {
    std::ifstream file("highscore.dat");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

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
    glColor3f(0.8f, 0.8f, 1.0f);
    sprintf(buf, "High Score: %d", highScore);
    glRasterPos2f(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT - 20);
    for (char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}


void saveHighScore() {
    std::ofstream file("highscore.dat");
    if (file.is_open()) {
        file << highScore;
        file.close();
    }
}

void updateHighScore() {
    if (score > highScore) {
        highScore = score;
        saveHighScore();
    }
}
