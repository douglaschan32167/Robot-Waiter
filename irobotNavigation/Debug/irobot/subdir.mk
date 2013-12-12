################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../irobot/irobot.c \
../irobot/irobotActuator.c \
../irobot/irobotCommand.c \
../irobot/irobotError.c \
../irobot/irobotSensor.c \
../irobot/irobotSensorPoll.c \
../irobot/irobotSensorStream.c \
../irobot/irobotTime.c \
../irobot/irobotUART.c \
../irobot/xqueue.c 

OBJS += \
./irobot/irobot.o \
./irobot/irobotActuator.o \
./irobot/irobotCommand.o \
./irobot/irobotError.o \
./irobot/irobotSensor.o \
./irobot/irobotSensorPoll.o \
./irobot/irobotSensorStream.o \
./irobot/irobotTime.o \
./irobot/irobotUART.o \
./irobot/xqueue.o 

C_DEPS += \
./irobot/irobot.d \
./irobot/irobotActuator.d \
./irobot/irobotCommand.d \
./irobot/irobotError.d \
./irobot/irobotSensor.d \
./irobot/irobotSensorPoll.d \
./irobot/irobotSensorStream.d \
./irobot/irobotTime.d \
./irobot/irobotUART.d \
./irobot/xqueue.d 


# Each subdirectory must supply rules for building sources it contributes
irobot/%.o: ../irobot/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -DMyRio_1950 -I"C:\Users\dougl_000\workspace\C Support for NI myRIO\source" -I"C:\Users\dougl_000\CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_GNU_Linux\arm-none-linux-gnueabi\libc\lib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


