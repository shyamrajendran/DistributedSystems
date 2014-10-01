################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../app.c \
../emulnet.c \
../log.c \
../mp1_node.c \
../params.c \
../queue.c 

OBJS += \
./app.o \
./emulnet.o \
./log.o \
./mp1_node.o \
./params.o \
./queue.o 

C_DEPS += \
./app.d \
./emulnet.d \
./log.d \
./mp1_node.d \
./params.d \
./queue.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


