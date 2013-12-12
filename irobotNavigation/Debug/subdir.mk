################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Accelerometer.c \
../UART.c \
../irobotNavigationStatechart.c \
../main.c 

OBJS += \
./Accelerometer.o \
./UART.o \
./irobotNavigationStatechart.o \
./main.o 

C_DEPS += \
./Accelerometer.d \
./UART.d \
./irobotNavigationStatechart.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -DMyRio_1950 -I"C:\Users\dougl_000\workspace\C Support for NI myRIO\source" -I"C:\Users\dougl_000\CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_GNU_Linux\arm-none-linux-gnueabi\libc\lib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


