#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "MyRio.h"
#include "Accelerometer.h"
#include "UART.h"
#include "irobot/irobot.h"
#include "irobotNavigationStatechart.h"
#include "DIO.h"
#include "AIO.h"

/* Function prototypes */
void rroll(const irobotSensorGroup6_t * const pSensors, const irobotUARTPort_t port);
uint64_t getTimeInMs(void);
void delayMs(const uint64_t msDelay);
void waitUntilNextMsMultiple(const uint64_t msMultiple);

/* program parameters */
const int32_t driveDistance = 200;		/* distance to drive, in mm */
const int32_t turnAngle = 90;			/* angle to turn, in mm */
const int16_t WHEEL_SPEED = 200;		/* maximum speed to turn the wheels, in mm/s */

int main(int argc, char **argv)
{
	/* Hardware peripherals */
	MyRio_Accl				accelDevice;
	irobotUARTPort_t 		port = UART1;			/* UART port */

	/* sensor inputs */
	irobotSensorGroup6_t	sensors;				/* irobot sensors */
	int32_t					netDistance;			/* net distance the robot has traveled, in mm */
	int32_t					netAngle;				/* net angle through which the robot has turned, in deg */
	accelerometer_t			accelValue = {0,0,0};	/* accelerometer, in g */

	/* actuator outputs */
	int16_t					leftWheelSpeed = 0;		/* speed of the left wheel, in mm/s */
	int16_t					rightWheelSpeed = 0;	/* speed of the right wheel, in mm/s */

	/* current state */
	int32_t					state = 0;				/* current state, as output by the Statechart */

    NiFpga_Status 			status;

    NiFpga_Bool dio_A0;
    MyRio_Dio A0;
    A0.dir = 0; //0 is input, 1 is output
    // A value of 0 means a low voltage (0 volts) is present,
    // a value of 1 means a high voltage (3.3 volts) is present.
    A0.in = DIOA_70IN;
    A0.out = DIOA_70OUT;
    A0.bit = 0;

    NiFpga_Bool dio_A1;
    MyRio_Dio A1;
    A1.dir = 0; //0 is input, 1 is output
    // A value of 0 means a low voltage (0 volts) is present,
    // a value of 1 means a high voltage (3.3 volts) is present.
    A1.in = DIOA_70IN;
    A1.out = DIOA_70OUT;
    A1.bit = 1;

    NiFpga_Bool dio_A2;
    MyRio_Dio A2;
    A2.dir = 0; //0 is input, 1 is output
    // A value of 0 means a low voltage (0 volts) is present,
    // a value of 1 means a high voltage (3.3 volts) is present.
    A2.in = DIOA_70IN;
    A2.out = DIOA_70OUT;
    A2.bit = 2;

    status = MyRio_Open();
	if (MyRio_IsNotSuccess(status)){
		MyRio_PrintStatus(status);
		return status;
	}


    //dio_A0 = Dio_ReadBit(&A0);
    //dio_A7 = Dio_ReadBit(&A7);

	/*
	 * Specify the registers that correspond to the accelerometer channel
	 * that needs to be accessed.
	 */
	accelDevice.xval = ACCXVAL;
	accelDevice.yval = ACCYVAL;
	accelDevice.zval = ACCZVAL;
	accelDevice.scale_wght = ACCSCALEWGHT;
	Accel_Scaling(&accelDevice);

	/* initialize iRobot */
	NiFpga_IfIsNotError(status, irobotOpen(port));

	/* Read inputs, execute statechart, generate outputs, print debug information */
	while(!NiFpga_IsError(status) && !sensors.buttons.advance){
		/* Read iRobot sensors */
		NiFpga_IfIsNotError(status, irobotSensorPollSensorGroup6(port, &sensors));
		if(NiFpga_IsNotError(status)){
			/* accumulate distance and angle */
			netDistance += sensors.distance;
			netAngle += sensors.angle;
		}

		/* Read and filter accelerometer */
		if(NiFpga_IsNotError(status)){
			accelValue.x = Accel_ReadX(&accelDevice);
			accelValue.y = Accel_ReadY(&accelDevice);
			accelValue.z = Accel_ReadZ(&accelDevice);
		}

	    dio_A0 = Dio_ReadBit(&A0);
	    dio_A1 = Dio_ReadBit(&A1);
	    dio_A2 = Dio_ReadBit(&A2);
		/* Execute statechart */
		irobotNavigationStatechart(
			WHEEL_SPEED,
			netDistance,
			netAngle,
			sensors,
			accelValue,
			false,
			&leftWheelSpeed,
			&rightWheelSpeed,
			&state,
			dio_A0,
			dio_A1,
			dio_A2
		);

		/* Produce outputs */
		NiFpga_IfIsNotError(status, irobotDriveDirect(port, leftWheelSpeed, rightWheelSpeed));

		/* print debug information */
		printf("\n\nx=%+.2f y=%+.2f z=%+.2f\nLWheel=%+3d RWheel=%+3d State=%+2d\n A2=%d\n A1=%d A0=%d",
				accelValue.x,
				accelValue.y,
				accelValue.z,
				leftWheelSpeed,
				rightWheelSpeed,
				state,
				dio_A2,
				dio_A1,
				dio_A0);
		//printf("%d\n",dio_A7);
		/* try uncommenting this line */
		/* rroll(&sensors, port); */

		/* Loop timing */
		waitUntilNextMsMultiple(60);
	}

	/* even if an error has occurred, close the UART port */
	NiFpga_MergeStatus(&status, irobotClose(port));

    MyRio_Close();

    /* Print status to the console */
    MyRio_PrintStatus(status);

    return status;
}

/* system time, in ms */
uint64_t getTimeInMs(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

/* delay a fixed number of ms */
void delayMs(const uint64_t msDelay){
	usleep(msDelay * 1000);
}

/* loop timing - wait until system clock is modulo ms */
void waitUntilNextMsMultiple(const uint64_t msMultiple){
	const uint64_t msCounter = getTimeInMs() % msMultiple;
	if(msCounter > 0){
		delayMs(msMultiple - msCounter);	/* delay modulo difference */
	}
}

/* sensor roll */
void rroll(const irobotSensorGroup6_t * const pSensors, const irobotUARTPort_t port){
	static uint8_t bInitialized = 0;

	if(!bInitialized){
		bInitialized = 1;

		/* initialize rr */
		const uint8_t rrInit[111] = {140,0,54,72,8,74,8,77,8,74,8,81,
			32,81,32,79,32,72,8,74,8,77,8,74,8,79,32,79,32,77,32,72,8,
			74,8,77,8,74,8,77,32,79,16,76,24,74,8,72,32,72,16,79,32,
			77,64,72,8,74,8,77,8,74,8,81,32,81,32,79,32,72,8,74,8,77,
			8,74,8,84,32,76,16,77,24,76,8,74,16,72,8,74,8,77,8,
			74,8,77,32,79,16,76,24,74,8,72,32,72,16,79,32,77,64};
		irobotUARTWriteRaw(port, rrInit, 111);
	}

	if(!pSensors->songPlaying){
		const uint8_t rr[2] = {141,0};
		irobotUARTWriteRaw(port, rr, 2);
	}
}
