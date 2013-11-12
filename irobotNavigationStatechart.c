/*
 * irobotNavigationStatechart.c
 *
 *  Created on: Mar 7, 2013
 *      Author:
 */

#include "irobotNavigationStatechart.h"
#include <math.h>
#include <stdlib.h>

/* Program States */
typedef enum{
	INITIAL = 0,						/* Initial state */
	PAUSE_WAIT_BUTTON_RELEASE,			/* Paused; pause button pressed down, wait until released before detecting next press */
	UNPAUSE_WAIT_BUTTON_PRESS,			/* Paused; wait for pause button to be pressed */
	UNPAUSE_WAIT_BUTTON_RELEASE,		/* Paused; pause button pressed down, wait until released before returning to previous state */
	DRIVE,								/* Drive straight */
	TURN,								/* Turn */
	STOP,
	FORWARD,
	BACKWARD,
	RIGHT,
	LEFT,
	MOVEMENT,
} robotState_t;

#define DEG_PER_RAD			(180.0 / M_PI)		/* degrees per radian */
#define RAD_PER_DEG			(M_PI / 180.0)		/* radians per degree */

#define GRID_DISTANCE  250

void irobotNavigationStatechart( //parameters
	const int16_t 				maxWheelSpeed,						/* (in) maximum wheel speed, in mm/s */
	const int32_t 				netDistance,						/* (in) net distance, in mm */
	const int32_t 				netAngle,							/* (in) net angle, in deg */
	const irobotSensorGroup6_t 	sensors,							/* (in) irobot sensors */
	const accelerometer_t 		accel,								/* (in) filtered accelerometer, in g */
	const bool					isSimulator,						/* (in) statechart is executing within a simulator */
	int16_t * const 			pRightWheelSpeed,					/*(out) right wheel speed */
	int16_t * const 			pLeftWheelSpeed,					/*(out) left wheel speed */
	int32_t * const				pState								/*(out) current state */
){ //body of function
	/* local state */
	static robotState_t 		state = INITIAL;				/* current program state */
	static robotState_t			unpausedState = DRIVE;			/* state history for pause region */
	static int32_t				distanceAtManeuverStart = 0;	/* distance robot had travelled when a maneuver begins, in mm */
	static int32_t				angleAtManeuverStart = 0;		/* angle through which the robot had turned when a maneuver begins, in deg */

	static int32_t				angleObjective = 0;
	//static char					direction = 1; //1 = straight, 2 = backwards, 3 = left, 4 = right
	static char					direction = 'N'; //'S','E','W'... N = North, S = South, E = East, W = West
	/* outputs */
	int16_t						leftWheelSpeed = 0;				/* speed of the left wheel, in mm/s */
	int16_t						rightWheelSpeed = 0;			/* speed of the right wheel, in mm/s */

	static int16_t				index = 0;
	static int					i;


	/******************************************************/
	/* state data - process inputs                        */
	/******************************************************/


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

	else if(state == DRIVE){
	  //if (detect obstacle){ go to MOVEMENT for new directions} 
	  if( abs(netDistance - distanceAtManeuverStart) >= GRID_DISTANCE){
	    STATE = MOVEMENT;
	  }
	}
	else if(state == FORWARD){
		if(direction == 'N'){
			state = DRIVE;
		}
		else if(direction == 'E'){
			if(abs(netAngle - angleAtManeuverStart) >= 90){
				direction = 'N';
			}
		}else if(direction == 'S'){
			if(abs(netAngle - angleAtManeuverStart) >= 90){
				direction = 'W';
			}
		}else if(direction == 'W'){
			if(abs(netAngle - angleAtManeuverStart) >= 90){
				direction = 'N';
			}
		}
	}
	else if(state == BACKWARD){
		if(direction == 'S'){
			state = DRIVE;
		}
		else if(direction == 'E'){
			if(abs(netAngle - angleAtManeuverStart) >= 90){
				direction = 'S'
			}
		}else if(direction == 'N'){
			if(abs(netAngle - angleAtManeuverStart) >= 90){
				direction = 'W';
			}
		}else if(direction == 'W'){
			if(abs(netAngle - angleAtManeuverStart) >= 90){
				direction = 'S';
			}
		}
	}
	else if(state == LEFT){
	  if(direction == 'W'){
	    state = DRIVE;
	  }
	  else if(direction == 'S'){
	    if(abs(netAngle - angleAtManeuverStart) >= 90){
	      direction = 'W';
	    }
	  }
	  else if(direction == 'N'){
	    if(abs(netAngle -angleAtManeuverStart) >= 90){
	      direction = 'W';
	    }
	  }
          else if(direction == 'E'){
            if(abs(netAngle -angleAtManeuverStart) >= 90){
	      direction= 'S';
            }
          }
	}

	else if(state == RIGHT){
          if(direction == 'E'){
            state = DRIVE;
          }
          else if(direction == 'S'){
            if(abs(netAngle - angleAtManeuverStart) >= 90){
              direction = 'E';
            }
          }
          else if(direction == 'N'){
            if(abs(netAngle -angleAtManeuverStart) >= 90){
              direction = 'E';
	    }
          }
          else if(direction == 'W'){
            if(abs(netAngle -angleAtManeuverStart) >= 90){
              direction= 'N';
            }
          }
	}

	else if(state == MOVEMENT){
	  //if(obstacle){ function that recalculates the path}
	  //array that has the path
	  //direction = set direction here from the calculated path;
	  //pop of the direction that is being used
	  if(direction == 'N'){
	    state = FORWARD;
	  }
	  else if(direction == 'S'){
	    state = BACKWARD;
	  }
	  else if(direction == 'E'){
	    state = RIGHT;
	  }
	  else if(direction == 'W'){
	    state = LEFT;
	  }
	}


	/* else, no transitions are taken */

	/*****************/
	/* state actions */
	/*****************/
	switch(state){
	case INITIAL:
	case PAUSE_WAIT_BUTTON_RELEASE:
	case UNPAUSE_WAIT_BUTTON_PRESS:
	case UNPAUSE_WAIT_BUTTON_RELEASE:
		/* in pause mode, robot should be stopped */
		leftWheelSpeed = rightWheelSpeed = 0;
		break;

	case DRIVE:
		/* full speed ahead! */
		leftWheelSpeed = rightWheelSpeed = maxWheelSpeed;
		break;
	case FORWARD:
	  if(direction == 'E'){
	    leftWheelSpeed = -maxWheelSpeed;
	    rightWheelSpeed = maxWheelSpeed;
	  }
	  else if(direction == 'S'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  else if(direction == 'W'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  break;
	case BACKWARD:
	  if(direction == 'E'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  else if(direction == 'W'){
	    leftWheelSpeed = -maxWheelSpeed;
	    rightWheelSpeed = maxWheelSpeed;
	  }
	  else if(direction = 'N'){
	    leftWheelSpeed = -maxWheelSpeed;
	    rightWheelSpeed = maxWheelSpeed;
	  }
	  break;
	case LEFT:
	  if(direction == 'N'){
	    leftWheelSpeed = -maxWheelSpeed;
	    rightWheelSpeed = maxWheelSpeed;
	  }
	  else if(direction == 'S'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  else if(direction == 'E'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  break;
	case RIGHT:
	  if(direction == 'S'){
	    leftWheelSpeed = -maxWheelSpeed;
	    rightWheelSpeed = maxWheelSpeed;
	  }
	  else if(direction == 'N'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  else if(direction == 'W'){
	    leftWheelSpeed = maxWheelSpeed;
	    rightWheelSpeed = -maxWheelSpeed;
	  }
	  break;
	case MOVEMENT:
	  leftWheelSpeed = 0;
	  rightWHeelSpeed = 0;
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
