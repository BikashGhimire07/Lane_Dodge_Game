
#ifndef VEHICLE_H
#define VEHICLE_H

#define NUM_OPPOSITE_CARS 6
#define NUM_LANES 4

enum VehicleType { VEHICLE_CAR, VEHICLE_BUS, VEHICLE_TRUCK };

struct Car {
    int lane;
    float x;
    float laneChangeProgress;
    int targetLane;
    bool isChangingLane;
    float y;
    float r, g, b;
    int direction;
    float speed;
    float lastSpeed;
    VehicleType type;
    float width, height;
    bool active;
    bool signalOn;
};

void initOppositeCars();
void drawOppositeCars();
void setVehicleType(Car& c);
void updateVehicleSpeedByType(Car& c);
extern Car oppositeCars[NUM_OPPOSITE_CARS];

#endif 

