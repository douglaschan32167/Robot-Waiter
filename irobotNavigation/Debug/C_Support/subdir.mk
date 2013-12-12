################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/dougl_000/workspace/C\ Support\ for\ NI\ myRIO/source/MyRio.c \
C:/Users/dougl_000/workspace/C\ Support\ for\ NI\ myRIO/source/NiFpga.c 

OBJS += \
./C_Support/MyRio.o \
./C_Support/NiFpga.o 

C_DEPS += \
./C_Support/MyRio.d \
./C_Support/NiFpga.d 


# Each subdirectory must supply rules for building sources it contributes
C_Support/MyRio.o: C:/Users/dougl_000/workspace/C\ Support\ for\ NI\ myRIO/source/MyRio.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -DMyRio_1950 -I"C:\Users\dougl_000\workspace\C Support for NI myRIO\source" -I"C:\Users\dougl_000\CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_GNU_Linux\arm-none-linux-gnueabi\libc\lib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

C_Support/NiFpga.o: C:/Users/dougl_000/workspace/C\ Support\ for\ NI\ myRIO/source/NiFpga.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -DMyRio_1950 -I"C:\Users\dougl_000\workspace\C Support for NI myRIO\source" -I"C:\Users\dougl_000\CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_GNU_Linux\arm-none-linux-gnueabi\libc\lib" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


