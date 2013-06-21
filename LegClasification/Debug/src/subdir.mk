################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Detector.cpp \
../src/clasificar.cpp \
../src/procesarDatos.cpp \
../src/svm.cpp 

OBJS += \
./src/Detector.o \
./src/clasificar.o \
./src/procesarDatos.o \
./src/svm.o 

CPP_DEPS += \
./src/Detector.d \
./src/clasificar.d \
./src/procesarDatos.d \
./src/svm.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ $(shell pkg-config --cflags mrpt-base mrpt-gui) -I/usr/include/mrpt/base/include/ -I/usr/include/mrpt/mrpt-config/ -I/usr/include/mrpt/gui/include/ -I/usr/include/mrpt/opengl/include/ -I/usr/include/mrpt/gui/ -I/usr/include -I../include -I/usr/include/mrpt/base/include -O0 -g3 -Wall -c -fmessage-length=0 -fpermissive -Wno-enum-compare -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


