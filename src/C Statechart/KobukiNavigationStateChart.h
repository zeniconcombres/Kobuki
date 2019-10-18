#ifndef KOBUKINAVIGATIONSTATECHART_H_
#define KOBUKINAVIGATIONSTATECHART_H_

#define _USE_MATH_DEFINES
#include "KobukiSensorType.h"

typedef enum {
	FORWARD,
	BACKWARD,
	ROTATE_RIGHT,
	ROTATE_LEFT,
	ARC,
	STOP
}drive_mode_t;

typedef enum {
	LEFT,
	CENTRE,
    CENTREX,
	RIGHT
}obstacle_loc_t;

typedef struct {
	drive_mode_t driveMode;
	double		 turnPct;
    bool         centreTurn; // default to 0 turn R
	int32_t		 distance;
	double       angle;
	obstacle_loc_t obstacleLoc;
	accelerometer_t offsets;
}variables_t;

/// Architecture-independent C Statechart.
void KobukiNavigationStatechart(
	const int16_t 				maxWheelSpeed,
	const int32_t 				netDistance,
	const int32_t 				netAngle,
	const KobukiSensors_t		sensors,
	const accelerometer_t		accelAxes,
	int16_t * const 			pRightWheelSpeed,
	int16_t * const 			pLeftWheelSpeed,
	const bool					isSimulator
);

typedef struct {
	const double inclineDetected;
	const double flatDetected;
	const double inclineIsNotForward;
	const double inclineIsForward;
	const double distanceReachedReverse;
	const double distanceReached;
}thresholds_t;

typedef struct {
	variables_t *			    variables;
	const int32_t 				netDistance;
	const int32_t 				netAngle;
	const KobukiSensors_t		sensors;
	const accelerometer_t		acc;
	const double				incline;
	const double				angle;
	const bool					isSimulator;
	const thresholds_t *		thresholds;
} system_t;

typedef struct {
	void(*actionFunc)(const system_t *);
} action_t;


struct state;
struct state_controller;

typedef struct {
	bool(*triggerFunc)(const system_t *);
	const struct state * newState;
	const size_t  numActions;
	const action_t * actions;
} transition_t;

typedef struct state {
	//void (*onStart)();
	//void (*eachRound)();
	//void * stateData;
	char* name;
	size_t numTransitions;
	transition_t* transitions;
	struct state_controller * nestedStateController;
}state_t;

typedef struct state_controller {
	const state_t * startState;
	state_t * currentState;
	variables_t  variables;
	const size_t  numActions;
	const action_t * initialActions;
}state_controller_t;

void controlSequence(state_controller_t * controller, const system_t * system);

void resetAll(const system_t * system);
void resetController(state_controller_t * controller);
void orientStart();

void changeState();

void setObstacleLocLeft(const system_t * system);
void setObstacleLocRight(const system_t * system);
void setObstacleLocCentre(const system_t * system);
void resetDistance(const system_t * system);
void resetAngle(const system_t * system);
void zeroAxes(const system_t * system);
double calculateIncline(const accelerometer_t * acc, const accelerometer_t * offset, const bool isSimulator);
double calculateAngle(const accelerometer_t * acc, const accelerometer_t * offset, const bool isSimulator);
void stop(const system_t * system);
void forward(const system_t * system);
void reverse(const system_t * system);
void rotateLeft(const system_t * system);
void rotateRight(const system_t * system);
void rotateToAvoid(const system_t * system);
void rotateToOrig(const system_t * system);
void ascendHill(const system_t * system);
void descendHill(const system_t * system);

bool triggerTrue(const system_t * system);
bool angleReached(const system_t * system);
bool distanceReachedReverse(const system_t * system);
bool distanceReached(const system_t * system);
bool inclineIsRight(const system_t * system);
bool inclineIsLeft(const system_t * system);
bool inclineIsFoward(const system_t * system);
bool inclineDetected(const system_t * system);
bool flatDetected(const system_t * system);

bool resetButtonPressed(const system_t * system);
bool pauseButtonPressed(const system_t * system);
bool pauseButtonReleased(const system_t * system);

bool obstacleDetected(const system_t * system);
bool obstacleDetectedLeft(const system_t * system);
bool obstacleDetectedRight(const system_t * system);


void drive(drive_mode_t driveMode, int16_t * pSpeedR, int16_t * pSpeedL, double turnPct);

//hill states
state_t groundState;
state_t ascendingState;
state_t adjustAscState;
state_t topState;
state_t descendingState;
state_t adjustDesState;
state_t endState;

//avoidance states
state_t driveState;
state_t reverseState;
state_t turnAwayState;
state_t turnBackState;
state_t driveAvoidState;

//pause states
state_t pauseWaitButtonReleaseState;
state_t unpauseWaitButtonReleaseState;
state_t unpauseWaitButtonPressState;
state_t runState;

#endif // IROBOTNAVIGATIONSTATECHART_H_
