#define NUM_LANE_DIVIDERS 15
struct LaneDivider {
    float x, y;
};
extern LaneDivider laneDividers[NUM_LANE_DIVIDERS];
void updateLaneDividers();
void drawRoadSides();
void drawLaneDividers();
#include "vehicle.h"

#pragma once

#define NUM_LANE_DIVIDERS 15
#define LANE_WIDTH 120
#define ROAD_WIDTH (NUM_LANES * LANE_WIDTH)
#define ROAD_LEFT ((SCREEN_WIDTH - ROAD_WIDTH) / 2)
#define ROAD_RIGHT (ROAD_LEFT + ROAD_WIDTH)

void initLaneDividers();
void drawRoadSides();
void drawLaneDividers();
