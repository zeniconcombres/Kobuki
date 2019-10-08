#ifndef KOBUKINAVIGATIONSTATECHART_H_
#define KOBUKINAVIGATIONSTATECHART_H_

#define _USE_MATH_DEFINES
#include "KobukiSensorType.h"

typedef enum {
	FORWARD,
	BACKWARD,
	ROTATE_RIGHT,
	ROTATE_LEFT,
	STOP
}drive_mode_t;

/// Architecture-independent C Statechart.
void KobukiNavigationStatechart(
	const int16_t 				maxWheelSpeed,
	const int32_t 				netDistance,
	const int32_t 				netAngle,
	const KobukiSensors_t		sensors,
	const accelerometer_t			accelAxes,
	int16_t * const 			pRightWheelSpeed,
	int16_t * const 			pLeftWheelSpeed,
	const bool					isSimulator
	);

typedef struct {
	const int32_t 				netDistance;
	const int32_t 				netAngle;
	const KobukiSensors_t		sensors;
	const accelerometer_t		accelAxes;
} system_t;

typedef struct {
	int16_t * wheel;
	int32_t speed;
} action_t;

typedef struct {
	bool(*triggerFunc)(const system_t *);
	const drive_mode_t drive_mode;
//	const size_t  numActions;
//	const action_t * actions;
} trigger_t;

struct state;

typedef struct {
	const trigger_t * trig;
	const struct state * newState;
} transition_t;

typedef struct state{
	//void (*onStart)();
	//void (*eachRound)();
	//void * stateData;
	size_t numTransitions;
	transition_t* transitions;
}state_t;

void orientStart();

void changeState();

bool resetButtonPressed(const system_t * system);
bool pauseButtonPressed(const system_t * system);

bool obstacleDetected(const system_t * system);
bool obstacleDetectedLeft(const system_t * system);
bool obstacleDetectedRight(const system_t * system);

void drive(drive_mode_t driveMode);

#endif // IROBOTNAVIGATIONSTATECHART_H_
