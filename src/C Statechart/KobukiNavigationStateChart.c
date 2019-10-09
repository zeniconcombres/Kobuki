/*
 * KobukiNavigationStatechart.c
 *
 */

#include "kobukiNavigationStatechart.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define TRANSITIONS(num) (transition_t[num])
#define ACTIONS(num)  (action_t[num])


state_t driveState = {
	1,
	TRANSITIONS(1){ 
		{&obstacleDetected, &reverseState, 2, ACTIONS(2) { {&reverse}, {&resetDistance} } }
	},
	NULL
};

state_t reverseState = {
	1,
	TRANSITIONS(1){
		{&distanceReached, &turnLeftState, 2, ACTIONS(2) { {&rotateLeft}, {&resetAngle} } }
	},
	NULL
};

state_t turnRightState = {
	0,
	NULL,
	NULL
};

state_t turnLeftState = {
	0,
	NULL,
	NULL
};

state_t driveAvoidState = {
	0,
	NULL,
	NULL
};

state_controller_t avoidanceController = {
	&driveState,
	NULL,
	{ STOP, 0, 0 },
	1,
	ACTIONS(1) { {&forward} }
};

state_t unPauseState = {
	1,
	TRANSITIONS(1){ 
		{&pauseButtonPressed, &pauseState, 1, ACTIONS(1){ {&stop } } }
	},
	&avoidanceController,
};

state_t pauseState = {
	1,
	TRANSITIONS(1){ {&pauseButtonPressed, &unPauseState} },
	NULL
};

state_controller_t mainController = {
	&pauseState,
	NULL,
	{STOP, 0, 0},
	0,
	NULL
};


variables_t variables = {
	STOP,
	0,
	0
};

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
                                ){
	const system_t system = { &variables, netDistance, netAngle, sensors, accelAxes };
	controlSequence(&mainController, &system);
	//current_state->onStart();
	//current_state->eachRound();
	//current_state->transitions.

	//set wheel speeds
	drive(variables.driveMode, pRightWheelSpeed, pLeftWheelSpeed);
}


void controlSequence(state_controller_t * controller, const system_t * system)
{
	if (controller->currentState == NULL)
	{
		for (int j = 0; j < controller->numActions; j++)
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
	for (int i = 0; i < current_state->numTransitions; i++)
	{
		if (current_state->transitions[i].triggerFunc(system))
		{
			const transition_t transition = current_state->transitions[i];
			for (int j = 0; j < transition.numActions; j++)
			{
				transition.actions[j].actionFunc(system);
			}
			controller->currentState = transition.newState;
			break;
		}	
	}
	//save state
	controller->variables = *system->variables;
}


void orientStart()
{

}

void changeState()
{

}

bool distanceReached(const system_t * system)
{
	return fabs(system->netDistance - system->variables->distance) > 100;
}


bool angleReached(const system_t * system)
{
	return fabs(system->netAngle - system->variables->angle) > M_PI_2;
}


bool resetButtonPressed(const system_t * system)
{
	return system->sensors.buttons.B1;
}


bool pauseButtonPressed(const system_t * system)
{
	return system->sensors.buttons.B0;
}

bool obstacleDetectedLeft(const system_t * system)
{
	return	system->sensors.cliffLeft					|| 
			system->sensors.bumps_wheelDrops.bumpLeft	|| 
			system->sensors.bumps_wheelDrops.wheeldropLeft;
}

bool obstacleDetectedRight(const system_t * system)
{
	return	system->sensors.cliffRight					|| 
			system->sensors.bumps_wheelDrops.bumpRight	|| 
			system->sensors.bumps_wheelDrops.wheeldropRight;
}

bool obstacleDetected(const system_t * system)
{
	return	obstacleDetectedLeft(system)	|| 
			obstacleDetectedRight(system)	|| 
			system->sensors.cliffCenter		|| 
			system->sensors.bumps_wheelDrops.bumpCenter;
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


void drive(drive_mode_t driveMode, int16_t * pSpeedR, int16_t * pSpeedL)
{
	const int16_t speed = 100;
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
	case STOP:
	default:
		speedL = speedR = 0;
	}
	*pSpeedR = speedR;
	*pSpeedL = speedL;
}
