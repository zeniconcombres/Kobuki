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

//const state_t state = {
//	&orientStart,
//	NULL,
//	NULL,
//	1,
//	TRIGGERS(1){ 10, 1, ACTIONS(1){ {1,1}}}
//};

const trigger_t pauseTrigger = {
	&pauseButtonPressed,
	STOP
};

const trigger_t unPauseTrigger = {
	&pauseButtonPressed,
	FORWARD
};
state_t pauseState;
state_t unPauseState = {
	1,
	TRANSITIONS(1){ {&pauseTrigger, &pauseState} }
};

state_t pauseState = {
	1,
	TRANSITIONS(1){ {&unPauseTrigger, &unPauseState} }
};


state_t * current_state = &pauseState;
int16_t speedL = 0;
int16_t speedR = 0;

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
	const system_t system = { netDistance, netAngle, sensors, accelAxes };
	for (int i = 0; i < current_state->numTransitions; i++)
	{
		if (current_state->transitions[i].trig->triggerFunc(&system))
		{
			printf("making transition\n");
			drive(current_state->transitions[i].trig->drive_mode);
			current_state = current_state->transitions[i].newState;
			break;
		}
	}

	//current_state->onStart();
	//current_state->eachRound();
	//current_state->transitions.
	
	//set wheel speeds
	*pRightWheelSpeed = speedR;
	*pLeftWheelSpeed = speedL;
}


void orientStart()
{

}

void changeState()
{

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



void drive(drive_mode_t driveMode)
{
	const int16_t speed = 100;
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
}
