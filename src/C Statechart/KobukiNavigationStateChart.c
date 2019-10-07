/*
 * KobukiNavigationStatechart.c
 *
 */

#include "kobukiNavigationStatechart.h"
#include <math.h>
#include <stdlib.h>


// Program States
typedef enum{
    INITIAL = 0,                            // Initial state
    PAUSE_WAIT_BUTTON_RELEASE,              // Paused; pause button pressed down, wait until released before detecting next press
    UNPAUSE_WAIT_BUTTON_PRESS,              // Paused; wait for pause button to be pressed
    UNPAUSE_WAIT_BUTTON_RELEASE,            // Paused; pause button pressed down, wait until released before returning to previous state
    DRIVE,                                  // Drive straight
    REVERSE,                                // Reverse straight
    TURN_RIGHT,                             // Turn right
    TURN_LEFT,                              // Turn left
    DRIVE_AVOID,                            // Drive after reverse and turning from object
    ROTATE_LEFT,                            // Rotating machine left to directly climb
    ROTATE_RIGHT,                           // Rotating machine right to directly climb
    TURNAROUND,                             // Turn 180 degrees after hitting top of ramp
    STOP                                    // Robot stops after descending hill
} robotState_t;

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
    
    // local state
    static robotState_t           state = INITIAL;                // current program state
    static robotState_t           unpausedState = DRIVE;            // state history for pause region
    static int32_t                distanceAtManeuverStart = 0;    // distance robot had travelled when a maneuver begins, in mm
    static int32_t                angleAtManeuverStart = 0;        // angle through which the robot had turned when a maneuver begins, in deg
    static int32_t                accelerometerZ = 0;       // accelerometer reading for Z at the very
    
    // outputs
    int16_t                       leftWheelSpeed = 0;                // speed of the left wheel, in mm/s
    int16_t                       rightWheelSpeed = 0;            // speed of the right wheel, in mm/s
    
    // speed constants
    int16_t                     speed = 100;                    // overall running speed of the robot, in mm/s
    
    // direction variables
    static int16_t              right = 0;                      // any right sensor is triggered in obstacle avoidance
    static int16_t              left = 0;                       // any left sensor is triggered in obstacle avoidance
    static int16_t              central = 0;                    // any central sensor is triggered in obstacle avoidance
    static int16_t              hillClimb = 0;                       // to denote if robot is on a hill climb
    static int16_t              flat = 0;                     // to denote if robot is not on a hill climb
    static int16_t              avoided = 0;                    // registering obstacle avoidance
    
    // sensor thresholds
    float                     sensorDiff = 0.05;               // 5 percent off from given value
    
    //*****************************************************
    // state data - process inputs                        *
    //*****************************************************
    
    // ***** NOTES from code: *****
    // maybe we should initialise the accelerometer Z axis readings at the start? - kind of have done this
    // could consider fine tuning the side (R/L) sensors because they never seem to be benefitting from a 90 degrees turn
    // for object avoidance? especially if there are things on the hill
    
    if (state == INITIAL
        || state == PAUSE_WAIT_BUTTON_RELEASE
        || state == UNPAUSE_WAIT_BUTTON_PRESS
        || state == UNPAUSE_WAIT_BUTTON_RELEASE
        || sensors.buttons.B0                // pause button
        ){
        switch (state){
            case INITIAL:
                // set state data that may change between simulation and real-world
                if (isSimulator){
                }
                else{
                }
                state = UNPAUSE_WAIT_BUTTON_PRESS; // place into pause state
                accelerometerZ = accelAxes.z; // flat ground z axis reading
                flat = 1; // default start assuming flat ground
                break;
            case PAUSE_WAIT_BUTTON_RELEASE:
                // remain in this state until released before detecting next press
                if (!sensors.buttons.B0){
                    state = UNPAUSE_WAIT_BUTTON_PRESS;
                }
                break;
            case UNPAUSE_WAIT_BUTTON_RELEASE:
                // user pressed 'pause' button to return to previous state
                if (!sensors.buttons.B0){
                    state = unpausedState; // this at the start transitions to DRIVE
                }
                break;
            case UNPAUSE_WAIT_BUTTON_PRESS:
                // remain in this state until user presses 'pause' button
                if (sensors.buttons.B0){
                    state = UNPAUSE_WAIT_BUTTON_RELEASE;
                }
                break;
            default:
                // must be in run region, and pause button has been pressed
                unpausedState = state;
                state = PAUSE_WAIT_BUTTON_RELEASE;
                break;
        }
    }
    //*************************************
    // state transition - obstacle avoidance region      *
    //*************************************
    else if (state == REVERSE
             || state == TURN_RIGHT
             || state == TURN_LEFT
             || state == DRIVE_AVOID
             || sensors.bumps_wheelDrops.bumpLeft
             || sensors.bumps_wheelDrops.bumpRight
             || sensors.bumps_wheelDrops.bumpCenter
             || sensors.cliffLeft
             || sensors.cliffRight
             || sensors.cliffCenter
             // hill climb states
             || state == ROTATE_LEFT
             || state == ROTATE_RIGHT
             || state == TURNAROUND){
        
        // registering which sensor is triggered - although multiple sensors? L + R + C?
        if (sensors.bumps_wheelDrops.bumpRight || sensors.cliffRight) {
            left = 0;
            right = 1;
            central = 0;
            angleAtManeuverStart = netAngle;
            distanceAtManeuverStart = netDistance;
            state = REVERSE;
        }
        else if (sensors.bumps_wheelDrops.bumpLeft || sensors.cliffLeft) {
            left = 1;
            right = 0;
            central = 0;
            angleAtManeuverStart = netAngle;
            distanceAtManeuverStart = netDistance;
            state = REVERSE;
        }
        else if (sensors.bumps_wheelDrops.bumpCenter || sensors.cliffCenter) {
            left = 0;
            right = 0;
            central = 1;
            angleAtManeuverStart = netAngle;
            distanceAtManeuverStart = netDistance;
            state = REVERSE;
        }
        // no sensor registered otherwise - probably can simplify this code...
        
        switch (state) {
            case REVERSE:
                // move on to turning
                if (abs(netDistance - distanceAtManeuverStart) >= 100){
                    if (central || left){
                            // default is to turn right
                            state = TURN_RIGHT;
                    } else {
                            // left turn
                            state = TURN_LEFT;
                    }
                    angleAtManeuverStart = netAngle;
                    distanceAtManeuverStart = netDistance;
                }
                break;
            case TURN_RIGHT:
            case TURN_LEFT:
                // after reverse, turning right or left will drive to avoid obstacle after
                if (abs(netAngle - angleAtManeuverStart) >= 90) {
                    angleAtManeuverStart = netAngle;
                    distanceAtManeuverStart = netDistance;
                    if (!avoided) {
                        state = DRIVE_AVOID;
                    } else {
                        avoided = 0;
                        state = DRIVE;
                    }
                }
                break;
            case DRIVE_AVOID:
                // turn back to original trajectory direction
                if (abs(netDistance - distanceAtManeuverStart) >= 100){
                    if (central || left){
                        // turned right now turning back left
                        avoided = 1;
                        state = TURN_LEFT;
                    }
                    else {
                        // turned left now turning back right
                        avoided = 1;
                        state = TURN_RIGHT;
                    }
                    angleAtManeuverStart = netAngle;
                    distanceAtManeuverStart = netDistance;
                }
                break;
            case ROTATE_LEFT:
            case ROTATE_RIGHT:
                if (abs(accelAxes.y) <= sensorDiff) {
                    angleAtManeuverStart = netAngle;
                    distanceAtManeuverStart = netDistance;
                    state = DRIVE;
                } // otherwise remain rotating until fixed
                break;
// Shouldn't need this for going up and down hills. Commented out //
//            case TURNAROUND:
//                // turning around completely after hitting wall keeping under 180 degrees turn
//                if (abs(netAngle - angleAtManeuverStart) >= 180) {
//                    angleAtManeuverStart = netAngle;
//                    distanceAtManeuverStart = netDistance;
//                    state = DRIVE;
//                } // otherwise remain rotating until fixed
//                break;
            default:
                angleAtManeuverStart = netAngle;
                distanceAtManeuverStart = netDistance;
                state = DRIVE;
                break;
        }
    }
    // else, no transitions are taken and the robot stays in DRIVE
    //*************************************
    // state transition - hill climb region      *
    //*************************************
    else if ((abs(accelAxes.z - accelerometerZ) >= sensorDiff*accelerometerZ)
             || abs(accelAxes.x) >= sensorDiff) {
        // branch with pause states and obstacle avoidance states
        switch (hillClimb) {
            case 0:
                // basically change from flat ground to register hill climb
                flat = 0;
                hillClimb = 1;
                if (accelAxes.y >= sensorDiff) {
                    // robot needs to turn right to correct left slant
                    state = ROTATE_RIGHT;
                } else if (accelAxes.y <= -sensorDiff) {
                    // robot needs to turn left to correct right slant
                    state = ROTATE_LEFT;
                }
                break;
            case 1:
                // on the hill transition to hill top or hill top to coming down
                if (flat == 1 && accelAxes.x <= sensorDiff) {
                    // going down
                    flat = 0;
                    hillClimb = 2;
                    if (accelAxes.y >= sensorDiff) {
                        // robot needs to turn left to correct right down slant
                        state = ROTATE_LEFT;
                    } else if (accelAxes.y <= -sensorDiff) {
                        // robot needs to turn right to correct left down slant
                        state = ROTATE_RIGHT;
                    }
                } else if (abs(accelAxes.z - accelerometerZ) <= sensorDiff*accelerometerZ
                           && abs(accelAxes.x) <= sensorDiff) {
                    // when we reach hill top we will then go down a ramp
                    flat = 1;
                }
                break;
            case 2:
                // robot stops after reaching ground from hill descent
                if (abs(accelAxes.z - accelerometerZ) <= sensorDiff*accelerometerZ
                    && abs(accelAxes.x) <= sensorDiff) {
                    flat = 1;
                    hillClimb = 0;
                    state = PAUSE_WAIT_BUTTON_RELEASE; // changed this to PAUSE for testing purposes
                }
                break;
        }
    }
    
    //*****************
    //* state actions *
    //*****************
    switch (state){
        case INITIAL:
        case PAUSE_WAIT_BUTTON_RELEASE:
        case UNPAUSE_WAIT_BUTTON_PRESS:
        case UNPAUSE_WAIT_BUTTON_RELEASE:
        case STOP:
            // in pause mode, robot should be stopped
            leftWheelSpeed = rightWheelSpeed = 0;
            break;
            
        case DRIVE:
        case DRIVE_AVOID:
            // full speed ahead!
            leftWheelSpeed = rightWheelSpeed = speed;
            break;
            
        case TURN_RIGHT:
        case ROTATE_RIGHT:
        case TURNAROUND:
            leftWheelSpeed = speed;
            rightWheelSpeed = -leftWheelSpeed;
            break;
            
        case TURN_LEFT:
        case ROTATE_LEFT:
            rightWheelSpeed = speed;
            leftWheelSpeed = -rightWheelSpeed;
            break;
            
        case REVERSE:
            // full speed back!
            leftWheelSpeed = rightWheelSpeed = -speed;
            break;
            
        default:
            // Unknown state
            leftWheelSpeed = rightWheelSpeed = 0;
            break;
    }
    
    
    *pLeftWheelSpeed = leftWheelSpeed;
    *pRightWheelSpeed = rightWheelSpeed;
}
