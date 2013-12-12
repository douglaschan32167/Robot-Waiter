#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "MyRio.h"
#include "Accelerometer.h"
#include "UART.h"
#include "irobot/irobot.h"
#include "irobotNavigationStatechart.h"

/* Function prototypes */
void rroll(const irobotSensorGroup6_t * const pSensors, const irobotUARTPort_t port);
uint64_t getTimeInMs(void);
void delayMs(const uint64_t msDelay);
void waitUntilNextMsMultiple(const uint64_t msMultiple);

/* program parameters */
const int32_t driveDistance = 200;		/* distance to drive, in mm */
const int32_t turnAngle = 90;			/* angle to turn, in mm */
const int16_t WHEEL_SPEED = 200;		/* maximum speed to turn the wheels, in mm/s */

typedef struct {
	char *directions;
} direction_data;

direction_data dir_data;

void *Get_Directions() {
	printf("start thread func");
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    char cmd = '0';
    time_t ticks;
	printf("start");

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    //memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        //ticks = time(NULL);
        //snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        //write(connfd, sendBuff, strlen(sendBuff));
		if (read(connfd, &cmd, 1) < 0) {
			printf("error in read");
		} else  {
			printf("got something");
			printf("%c", cmd);
			if (cmd == 'S') {
				dir_data.directions = '0';
			} else {
				dir_data.directions = cmd;
			}
		}
        close(connfd);
        sleep(1);
     }
	//char* testdirs = "NEESSWWNNWNS0";
	//dir_data.directions = 'R';//changes pointer
	//sleep(5);
	//dir_data.directions = '0';
	//sleep(4);
	//dir_data.directions = 'L';//changes pointer
	//sleep(5);
	//dir_data.directions = '0';
	pthread_exit(NULL);
}

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

    status = MyRio_Open();
    if (MyRio_IsNotSuccess(status)){
    	MyRio_PrintStatus(status);
        return status;
    }

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
	dir_data.directions = calloc(0, 20);

  	pthread_t ptt;
    int thread_start = pthread_create(&ptt, NULL, Get_Directions, NULL);
    if (thread_start != 0) {
    	printf("error creating thread");
    	exit(-1);
    }

	MyRio_PrintStatus(status);
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
			dir_data.directions
		);

		/* Produce outputs */
		NiFpga_IfIsNotError(status, irobotDriveDirect(port, leftWheelSpeed, rightWheelSpeed));

		/* print debug information */
		printf("\n\nx=%+.2f y=%+.2f z=%+.2f\nLWheel=%+3d RWheel=%+3d State=%+2d\n",
				accelValue.x,
				accelValue.y,
				accelValue.z,
				leftWheelSpeed,
				rightWheelSpeed,
				state);

		/* try uncommenting this line */
		 //rroll(&sensors, port);

		/* Loop timing */
		waitUntilNextMsMultiple(60);

		printf("end of loop");
		MyRio_PrintStatus(status);
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
