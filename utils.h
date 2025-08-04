 #ifndef UTILS_H
 #define UTILS_H

void playMP3(const char* alias, const char* filename, bool loop);
void stopMP3(const char* alias);
void handleAudio();
void mouseMenu(int button, int state, int x, int y);

float lanePosX(int lane, int offset, float carWidth = 50);
int checkCollision();
void drawCrashEffect();
void display();
void timer(int value);

extern float playerKmh;
extern float playerBaseSpeed;
extern float playerMinSpeed;
extern float playerMaxSpeed;
extern int playerLane;
extern int playerLaneOffset;
extern float playerY;
extern int score;
extern int highScore;
extern int lives;
extern int activeOppositeCars;
extern int trafficIncreaseTimer;
extern bool gameOver;
extern bool newHighScore;
extern int gameOverTimer;
extern const int GAME_OVER_DELAY;
extern bool playerCrashed;
extern float crashX, crashY;
extern int respawnTimer;
extern const int RESPAWN_DELAY;
extern bool playerVisible;
extern int crashedVehicleIdx;

#endif // UTILS_H