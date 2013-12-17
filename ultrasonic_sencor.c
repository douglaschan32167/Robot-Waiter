/*
 * irobotNavigationStatechart.c
 *
 *  Created on: Mar 7, 2013
 *      Author:
 */

#include "irobotNavigationStatechart.h"
#include <math.h>
#include <stdlib.h>
#include "DIO.h"
#include "myRIO.h"
#include "AIO.h"
/* Program States */
typedef enum{
	INITIAL = 0,						/* Initial state */
	PAUSE_WAIT_BUTTON_RELEASE,			/* Paused; pause button pressed down, wait until released before detecting next press */
	UNPAUSE_WAIT_BUTTON_PRESS,			/* Paused; wait for pause button to be pressed */
	UNPAUSE_WAIT_BUTTON_RELEASE,		/* Paused; pause button pressed down, wait until released before returning to previous state */
	DRIVE,								/* Drive straight */
	TURN,								/* Turn */
    FORWARD,                            /* Drive straight */
    TURNL,                                /* Turn */
    TURNR,                                /* Turn */
    LOCATE,                                /* Turn */
    WAIT,
	STOP
} robotState_t;

#define DEG_PER_RAD			(180.0 / M_PI)		/* degrees per radian */
#define RAD_PER_DEG			(M_PI / 180.0)		/* radians per degree */

void irobotNavigationStatechart(
	const int16_t 				maxWheelSpeed,						/* (in) maximum wheel speed, in mm/s */
	const int32_t 				netDistance,						/* (in) net distance, in mm */
	const int32_t 				netAngle,							/* (in) net angle, in deg */
	const irobotSensorGroup6_t 	sensors,							/* (in) irobot sensors */
	const accelerometer_t 		accel,								/* (in) filtered accelerometer, in g */
	const bool					isSimulator,						/* (in) statechart is executing within a simulator */
	int16_t * const 			pRightWheelSpeed,					/*(out) right wheel speed */
	int16_t * const 			pLeftWheelSpeed,					/*(out) left wheel speed */
	int32_t * const				pState,								/*(out) current state */
	const NiFpga_Bool           dio_A0,
	const NiFpga_Bool           dio_A1,
	const NiFpga_Bool           dio_A2
){
	/* local state */
	static robotState_t 		state = INITIAL;				/* current program state */
	static robotState_t			unpausedState = WAIT;			/* state history for pause region */
	static int32_t				distanceAtManeuverStart = 0;	/* distance robot had travelled when a maneuver begins, in mm */
	static int32_t				angleAtManeuverStart = 0;		/* angle through which the robot had turned when a maneuver begins, in deg */
    static int32_t 				distanceForward = 0;
    static robotState_t         previous = INITIAL;

	/* outputs */
	int16_t						leftWheelSpeed = 0;				/* speed of the left wheel, in mm/s */
	int16_t						rightWheelSpeed = 0;			/* speed of the right wheel, in mm/s */

	/******************************************************/
	/* state data - process inputs                        */
	/******************************************************/
    int recieveOrder = 1;
    int tableAligned = 1;


	/******************************************************/
	/* state transition - pause region (highest priority) */
	/******************************************************/
	if(   state == INITIAL
	   || state == PAUSE_WAIT_BUTTON_RELEASE
	   || state == UNPAUSE_WAIT_BUTTON_PRESS
	   || state == UNPAUSE_WAIT_BUTTON_RELEASE
	   || sensors.buttons.play				/* pause button */
	){
		switch(state){
		case INITIAL:
			/* set state data that may change between simulation and real-world */
			if(isSimulator){
			}
			else{
			}
			state = UNPAUSE_WAIT_BUTTON_PRESS; /* place into pause state */
			break;
		case PAUSE_WAIT_BUTTON_RELEASE:
			/* remain in this state until released before detecting next press */
			if(!sensors.buttons.play){
				state = UNPAUSE_WAIT_BUTTON_PRESS;
			}
			break;
		case UNPAUSE_WAIT_BUTTON_RELEASE:
			/* user pressed 'pause' button to return to previous state */
			if(!sensors.buttons.play){
				state = unpausedState;
			}
			break;
		case UNPAUSE_WAIT_BUTTON_PRESS:
			/* remain in this state until user presses 'pause' button */
			if(sensors.buttons.play){
				state = UNPAUSE_WAIT_BUTTON_RELEASE;
			}
			break;
		default:
			/* must be in run region, and pause button has been pressed */
			unpausedState = state;
			state = PAUSE_WAIT_BUTTON_RELEASE;

			break;
		}
	}
	/**************************************/
	/* state transition - run region      */
	/**************************************/
    else if(state == WAIT && recieveOrder){
    	state = DRIVE;
    }
    else if(state == LOCATE && tableAligned){
    	printf("\n dumb TURN");
    	//optitrack
    	state = DRIVE;
    }
    else if(state == DRIVE && dio_A0){
        if (dio_A1 && !dio_A2){
        	printf("\n LEFT TURN");
			state = TURNL;
			angleAtManeuverStart = netAngle;
		}else if (dio_A2  && !dio_A1){
        	printf("\n RIGHT TURN");
			state = TURNR;
			angleAtManeuverStart = netAngle;
		}else if (dio_A1 && dio_A2){
        	printf("\n WAIT");
			state = WAIT;
		}else{
        	printf("\n elseLEFT TURN");
			state = TURNL;
			angleAtManeuverStart = netAngle;
		}
    }
    else if(state == TURNR){
        if (abs(netAngle - angleAtManeuverStart) >= 90){
			angleAtManeuverStart = netAngle;
			previous = TURNR;
            state = FORWARD;
        }else if (abs(netAngle - angleAtManeuverStart) >= 90){
			angleAtManeuverStart = netAngle;
            state = LOCATE;
        }
        printf("\n turnr");
        distanceAtManeuverStart = netDistance;
    }
    else if(state == TURNL){
        if (abs(netAngle - angleAtManeuverStart) >= 90){
			angleAtManeuverStart = netAngle;
			previous = TURNL;
            state = FORWARD;
        }else if (abs(netAngle - angleAtManeuverStart) >= 90){
			angleAtManeuverStart = netAngle;
            state = LOCATE;
        }
        printf("\n turnl");
        distanceAtManeuverStart = netDistance;
    }
    else if(state == FORWARD
    		&& ((!dio_A1 && previous == TURNL) || (!dio_A2 && previous == TURNR))
    		&& distanceForward >= 250){
    	printf("\n forward");
        distanceForward = 0;
        state = LOCATE;
    }
    else if(state == FORWARD && !dio_A1 && previous == TURNL){
    	printf("\n forward");
        distanceForward = netDistance - distanceAtManeuverStart;
    }
    else if(state == FORWARD && !dio_A2 && previous == TURNR){
    	printf("\n forward");
        distanceForward = netDistance - distanceAtManeuverStart;
    }else if(state == WAIT){
    	printf("\n wait");
    	if (!dio_A0){
            state = LOCATE;
    	}else if (!dio_A1){
			state = TURNR;
			angleAtManeuverStart = netAngle;
		}else if (!dio_A2){
			state = TURNL;
			angleAtManeuverStart = netAngle;
		}
    }else if(state == FORWARD){
    	printf("\n forward");
    }

	/* else, no transitions are taken */

	/*****************/
	/* state actions */
	/*****************/
	switch(state){
	case INITIAL:
	case PAUSE_WAIT_BUTTON_RELEASE:
	case STOP:
	case UNPAUSE_WAIT_BUTTON_PRESS:
	case UNPAUSE_WAIT_BUTTON_RELEASE:
	case WAIT:
		/* in pause mode, robot should be stopped */
		leftWheelSpeed = rightWheelSpeed = 0;
		break;

	case DRIVE:
    case FORWARD:
		/* full speed ahead! */
		leftWheelSpeed = rightWheelSpeed = maxWheelSpeed;
		break;

	case TURN:
		leftWheelSpeed = maxWheelSpeed;
		rightWheelSpeed = -leftWheelSpeed;
		break;

    case LOCATE:
    case TURNL:
        leftWheelSpeed = -maxWheelSpeed;
        rightWheelSpeed = maxWheelSpeed;
        break;

    case TURNR:
        leftWheelSpeed = maxWheelSpeed;
        rightWheelSpeed = -maxWheelSpeed;
        break;

	default:
		/* Unknown state */
		leftWheelSpeed = rightWheelSpeed = 0;
		break;
	}

	/* write outputs */
	*pLeftWheelSpeed = leftWheelSpeed;
	*pRightWheelSpeed = rightWheelSpeed;
}
