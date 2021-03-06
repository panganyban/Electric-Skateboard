#include "ElectricBoardControl.hpp"
#include "lpc_pwm.hpp"
#include "shared_handles.h"
#include <stdio.h>
#include "queue.h"
#include "printf_lib.h"

ElectricBoardControl::ElectricBoardControl(uint8_t priority) :
    scheduler_task("ctrlbrd", 5000, priority, NULL) {
}

ElectricBoardControl::~ElectricBoardControl()
{
}

bool ElectricBoardControl::init(void)
{
    const unsigned int PWM_FREQ_HZ = 45;
    bool commandQueueShared = false;
    commandQueue = xQueueCreate(20, sizeof(int));
    if (commandQueue)
    {
        commandQueueShared = addSharedObject(shared_ElectricBoardQueue, commandQueue);
    }

    motorChannel1 = new PWM(PWM::pwm1, PWM_FREQ_HZ);
    motorChannel2 = new PWM(PWM::pwm2, PWM_FREQ_HZ);

    return commandQueue && commandQueueShared;
}

bool ElectricBoardControl::run(void *param)
{
    if (commandQueue == NULL)
    {
        return false;
    }

    // If no command is received in driveTimeout milliseconds, then the drive
    // will be set to 0. The trigger needs to be held to keep driving the motors.
    int driveLevel = 0;
    xQueueReceive(commandQueue, &driveLevel, driveTimeout);
    //u0_dbg_printf("Driving motors at %i % \n", driveLevel);
    driveMotors(driveLevel);
    return true;
}

void ElectricBoardControl::driveMotors(float powerLevel)
{
    // PWM Freq = 45 Hz

    if (powerLevel > 100) { powerLevel = 100; }
    else if (powerLevel < 0) { powerLevel = 0; }

    double dutyCycle = IDLE_DUTY + (MAX_DUTY - IDLE_DUTY) * (powerLevel / 100);
    u0_dbg_printf("Duty Cycle is %f % \n", dutyCycle);

    motorChannel1->set(dutyCycle);
    motorChannel2->set(dutyCycle);
}
