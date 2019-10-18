/*
 * KobukiNavigationStatechart.c
 *
 */

#include "kobukiNavigationStatechart.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define TRANSITIONS(num) num, (transition_t[num])
#define ACTIONS(num)  num, (action_t[num])

state_t groundState = {
	"Ground",
	TRANSITIONS(1) {
		{&inclineDetected, &ascendingState}
},
NULL
};

state_t ascendingState = {
	"Ascending",
	TRANSITIONS(2) {
		{&flatDetected, &topState, ACTIONS(1){ {&forward} }},
		{ &triggerTrue, &ascendingState, ACTIONS(1){ {&ascendHill} } }
},
NULL
};

//state_t adjustAscState = {
//	TRANSITIONS(1) {
//		{&inclineIsFoward, &ascendingState, ACTIONS(1){ {&forward}} }
//	},
//	NULL
//};

state_t topState = {
	"Top",
	TRANSITIONS(1) {
		{&inclineDetected, &descendingState}
},
NULL
};

state_t descendingState = {
	"Descending",
	TRANSITIONS(2) {
		{&flatDetected, &endState, ACTIONS(2){ {&resetAll},{ &stop } } },
		{ &triggerTrue, &descendingState, ACTIONS(1){ {&descendHill} } }
},
NULL
};

//state_t adjustDesState = {
//	TRANSITIONS(1) {
//		{&inclineIsFoward, &descendingState, ACTIONS(1){ {&forward}} }
//	},
//	NULL
//};

state_t endState = {
	"End",
	0,
	NULL,
	NULL
};

state_controller_t hillStateController = {
	&groundState,
	NULL,
{ STOP, 0, 0, 0, CENTRE,{ 0.0, 0.0, 0.0 } },
ACTIONS(2) { {&forward},{ &zeroAxes } }
};


state_t driveState = {
	"Drive",
	TRANSITIONS(3){
		{ &obstacleDetectedLeft,  &reverseState, ACTIONS(3) { {&reverse},{ &resetDistance },{ &setObstacleLocLeft } } },
		{ &obstacleDetectedRight, &reverseState, ACTIONS(3) { {&reverse},{ &resetDistance },{ &setObstacleLocRight } } },
		{ &obstacleDetected,      &reverseState, ACTIONS(3) { {&reverse},{ &resetDistance },{ &setObstacleLocCentre } } }
        // Central bumper always turns left
},
&hillStateController
};

state_t reverseState = {
	"Reverse",
	TRANSITIONS(1){
		{&distanceReachedReverse, &turnAwayState, ACTIONS(2) { {&rotateToAvoid},{ &resetAngle } } }
},
NULL
};

state_t turnAwayState = {
	"TurnAway",
	TRANSITIONS(1){
		{&angleReached, &driveAvoidState, ACTIONS(2) { {&forward},{ &resetDistance } } }
},
NULL
};

state_t turnBackState = {
	"TurnBack",
	TRANSITIONS(1){
		{&angleReached, &driveState, ACTIONS(2) { {&forward},{ &resetDistance } } }
},
NULL
};

state_t driveAvoidState = {
	"DriveAvoid",
	TRANSITIONS(2) {
		{&distanceReached, &turnBackState, ACTIONS(2) { {&rotateToOrig},{ &resetAngle } } },
		//{ &obstacleDetectedinAvoid, &reverseState, ACTIONS(3) { {&reverse},{ &resetDistance }, { &setCentreTurn } } }
		{ &obstacleDetected, &reverseCornerState, ACTIONS(3) { {&reverse},{ &resetDistance },{ &setCentreTurn } } }

        //TODO: define behaviour when find a corner
        // reverse + turn 180 + rotate back + if bumper center, turn other way.
},
NULL
};

state_t reverseCornerState = {
	"ReverseCorner",
	TRANSITIONS(1) {
		{&distanceReachedReverse, &turnBackState, ACTIONS(2) { {&rotateToAvoid},{ &resetAngle } } },
},
NULL
};

state_controller_t avoidanceController = {
	&driveState,
	NULL,
{ STOP, 0, 0, 0, CENTRE,{ 0.0, 0.0, 0.0 } },
ACTIONS(1) { {&forward} }
};

state_t runState = {
	"Run",
	TRANSITIONS(1) {
		{&pauseButtonPressed, &pauseWaitButtonReleaseState, ACTIONS(1){ {&stop} } }
},
&avoidanceController,
};

state_t pauseWaitButtonReleaseState = {
	"PauseWaitButtonRelease",
	TRANSITIONS(1) {
		{&pauseButtonReleased, &unpauseWaitButtonPressState},
},
NULL
};

state_t unpauseWaitButtonPressState = {
	"UnpauseWaitButtonPress",
	TRANSITIONS(1) {
		{&pauseButtonPressed, &unpauseWaitButtonReleaseState},
},
NULL
};

state_t unpauseWaitButtonReleaseState = {
	"UnpauseWaitButtonRelease",
	TRANSITIONS(1) {
		{&pauseButtonReleased, &runState},
},
NULL
};

//		{ &resetButtonPressed, &pauseState, ACTIONS(1){ {&resetAll} } }
state_controller_t mainController = {
	&unpauseWaitButtonPressState,
	NULL,
{ STOP, 0, 0, 0, CENTRE,{ 0.0, 0.0, 0.0 } },
NULL
};


variables_t variables = {
	STOP,
	0,
    0,
	0,
	CENTRE,
{ 0.0, 0.0, 0.0 }
};

thresholds_t simThresholds = {
	0.07,	// inclineDetected
	0.04,	// flatDetected
	0.25,	// inclineIsNotForward
	0.1,	// inclineIsForward
	100,	// distanceReachedReverse
	100		// distanceReachedSide
};

thresholds_t realThresholds = {
	0.2,	// inclineDetected
	0.15,	// flatDetected
	0.15,	// inclineIsNotForward
	0.1,	// inclineIsForward
	150,	// distanceReachedReverse
	100		// distanceReachedSide
};

bool         centreTurn = false;    // default to 0 turn R

#define DEG_PER_RAD            (180.0 / M_PI)        // degrees per radian
#define RAD_PER_DEG            (M_PI / 180.0)        // radians per degree

void KobukiNavigationStatechart(
	const int16_t                 maxWheelSpeed,
	const int32_t                 netDistance,
	const int32_t                 netAngle,
	const KobukiSensors_t         sensors,
	const accelerometer_t         accelAxes,
	int16_t * const               pRightWheelSpeed,
	int16_t * const               pLeftWheelSpeed,
	const bool                    isSimulator
) {
	const thresholds_t * thresholds;
	if (isSimulator)
	{
		thresholds = &simThresholds;
	}
	else
	{
		thresholds = &realThresholds;
	}
	const system_t system = { &variables,
		netDistance,
		netAngle,
		sensors,
		accelAxes,
		calculateIncline(&accelAxes, &variables.offsets, isSimulator),
		calculateAngle(&accelAxes, &variables.offsets, isSimulator),
		isSimulator,
		thresholds };
	controlSequence(&mainController, &system);
	//set wheel speeds
	drive(variables.driveMode, pRightWheelSpeed, pLeftWheelSpeed, variables.turnPct);
}


void controlSequence(state_controller_t * controller, const system_t * system)
{
	if (controller->currentState == NULL)
	{
		int j;
		for (j = 0; j < controller->numActions; j++)
		{
			controller->initialActions[j].actionFunc(system);
		}
		controller->currentState = controller->startState;
	}
	else
	{
		*system->variables = controller->variables;
	}

	const state_t * current_state = controller->currentState;
	if (current_state->nestedStateController != NULL)
	{
		controlSequence(current_state->nestedStateController, system);
	}
	int i;
	for (i = 0; i < current_state->numTransitions; i++)
	{
		if (current_state->transitions[i].triggerFunc(system))
		{
			const transition_t transition = current_state->transitions[i];
			int j;
			for (j = 0; j < transition.numActions; j++)
			{
				transition.actions[j].actionFunc(system);
			}
			printf("New State %s \n", transition.newState->name);
			printf("Angle %.2lf\n", system->angle);
			controller->currentState = transition.newState;
			break;
		}
	}
	//save state
	controller->variables = *system->variables;
}

void resetAll(const system_t * system)
{
	resetController(&avoidanceController);
	resetController(&mainController);
	resetController(&hillStateController);
}

void resetController(state_controller_t * controller)
{
	controller->currentState = NULL;
}


void orientStart()
{

}

void changeState()
{

}

bool triggerTrue(const system_t * system)
{
	return true;
}

bool distanceReached(const system_t * system)
{
	return abs(system->netDistance - system->variables->distance) > system->thresholds->distanceReached;
}

bool distanceReachedReverse(const system_t * system)
{
	return abs(system->netDistance - system->variables->distance) > system->thresholds->distanceReachedReverse;
}


bool angleReached(const system_t * system)
{
    return abs(system->netAngle - system->variables->angle) >= 90;
}


bool resetButtonPressed(const system_t * system)
{
	return system->sensors.buttons.B1;
}


bool pauseButtonPressed(const system_t * system)
{
	return system->sensors.buttons.B0;
}

bool pauseButtonReleased(const system_t * system)
{
	return !pauseButtonPressed(system);
}

bool obstacleDetectedLeft(const system_t * system)
{
	return	system->sensors.cliffLeft ||
		system->sensors.bumps_wheelDrops.bumpLeft;//	||
												  //system->sensors.bumps_wheelDrops.wheeldropLeft;
}

bool obstacleDetectedRight(const system_t * system)
{
	return	system->sensors.cliffRight ||
		system->sensors.bumps_wheelDrops.bumpRight;//	||
												   //system->sensors.bumps_wheelDrops.wheeldropRight;
}

bool obstacleDetected(const system_t * system)
{
	return	obstacleDetectedLeft(system) ||
		obstacleDetectedRight(system) ||
		system->sensors.cliffCenter ||
		system->sensors.bumps_wheelDrops.bumpCenter;
}

bool obstacleDetectedinAvoid(const system_t * system)
{
    // reversing turn direction
    return    obstacleDetectedLeft(system) ||
        obstacleDetectedRight(system) ||
        system->sensors.cliffCenter ||
        system->sensors.bumps_wheelDrops.bumpCenter;
}

void setCentreTurn(const system_t * system)
{
	centreTurn = !(centreTurn);
}


void setObstacleLocLeft(const system_t * system)
{
	system->variables->obstacleLoc = LEFT;
}

void setObstacleLocRight(const system_t * system)
{
	system->variables->obstacleLoc = RIGHT;
}

void setObstacleLocCentre(const system_t * system)
{
	system->variables->obstacleLoc = CENTRE;
}

void resetDistance(const system_t * system)
{
	system->variables->distance = system->netDistance;
}

void resetAngle(const system_t * system)
{
	system->variables->angle = system->netAngle;
}

void forward(const system_t * system)
{
	system->variables->driveMode = FORWARD;
}

void reverse(const system_t * system)
{
	system->variables->driveMode = BACKWARD;
}

void rotateToAvoid(const system_t * system)
{
	if (system->variables->obstacleLoc == LEFT
        || centreTurn)
	{
		rotateRight(system);
	}
	else
	{
        // needs bool parameter to determine previous avoid
		rotateLeft(system);
	}
}

void rotateToOrig(const system_t * system)
{
	if (system->variables->obstacleLoc == LEFT
        || centreTurn)
	{
		rotateLeft(system);
	}
	else
	{
		rotateRight(system);
	}
}

void rotateRight(const system_t * system)
{
	system->variables->driveMode = ROTATE_RIGHT;
}

void rotateLeft(const system_t * system)
{
	system->variables->driveMode = ROTATE_LEFT;
}

void stop(const system_t * system)
{
	system->variables->driveMode = STOP;
}

bool inclineIsLeft(const system_t * system)
{
	return system->angle < -system->thresholds->inclineIsNotForward;
}

bool inclineIsRight(const system_t * system)
{
	return system->angle > system->thresholds->inclineIsNotForward;
}

bool inclineIsFoward(const system_t * system)
{
	return fabs(system->angle) < system->thresholds->inclineIsForward;
}

bool inclineDetected(const system_t * system)
{
	return fabs(system->incline) > system->thresholds->inclineDetected;
}

bool flatDetected(const system_t * system)
{
	return fabs(system->incline) < system->thresholds->flatDetected;
}

double calculateIncline(const accelerometer_t * acc, const accelerometer_t * offset, const bool isSimulator)
{
	double x = acc->x - offset->x;
	double y = acc->y - offset->y;
	double z = acc->z - offset->z;
	return acos(z / (sqrt(x*x + y * y + z * z)));
}

double calculateAngle(const accelerometer_t * acc, const accelerometer_t * offset, const bool isSimulator)
{
	double x = acc->x - offset->x;
	double y = acc->y - offset->y;
	if (isSimulator)
	{
		return atan2(x, y);
	}
	return atan2(-y, x);
}

void zeroAxes(const system_t * system)
{
	system->variables->offsets.x = system->acc.x;
	system->variables->offsets.y = system->acc.y;
	system->variables->offsets.z = 1.0 - system->acc.z;
}

void ascendHill(const system_t * system)
{
	system->variables->driveMode = ARC;
	system->variables->turnPct = -system->angle / M_PI;
}

void descendHill(const system_t * system)
{
	system->variables->driveMode = ARC;
	double angle = system->angle;
	if (angle>0)
	{
		system->variables->turnPct = -(angle - M_PI) / M_PI;
	}
	else
	{
		system->variables->turnPct = -(angle + M_PI) / M_PI;
	}
}

void drive(drive_mode_t driveMode, int16_t * pSpeedR, int16_t * pSpeedL, double turnPct)
{
	const int16_t speed = 200;
	int16_t speedR = 0;
	int16_t speedL = 0;
	switch (driveMode)
	{
	case FORWARD:
		speedR = speedL = speed;
		break;
	case BACKWARD:
		speedR = speedL = -speed;
		break;
	case ROTATE_LEFT:
		speedR = speed;
		speedL = -speed;
		break;
	case ROTATE_RIGHT:
		speedR = -speed;
		speedL = speed;
		break;
	case ARC:
		speedR = speed * (1.0 + turnPct);
		speedL = speed * (1.0 - turnPct);
		break;
	case STOP:
	default:
		speedL = speedR = 0;
	}
	*pSpeedR = speedR;
	*pSpeedL = speedL;
}
