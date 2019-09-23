/*
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include "Accelerometer.h"
#include "MyRio.h"

#if !defined(LoopDuration)
#define LoopDuration    60  /* How long to output the signal, in seconds */
#endif


#if !defined(LoopSteps)
#define LoopSteps       1   /* How long to step between printing, in seconds */
#endif


/**
 * Overview:
 * Demonstrates using the onboard accelerometer. Reads the acceleration in the
 * three directions. Prints the values to the console.
 *
 * Instructions:
 * 1. Run this program.
 * 2. Shake the NI myRIO.
 *
 * Output:
 * The program reads the acceleration for 60 s. Acceleration values are written
 * to the console every 1 s.
 *
 * Note:
 * The Eclipse project defines the preprocessor symbol for the NI myRIO-1900.
 * Change the preprocessor symbol to use this example with the NI myRIO-1950.
 */
int main(int argc, char **argv)
{
    NiFpga_Status status;

    MyRio_Accl Accl;

    time_t currentTime;
    time_t printTime;
    time_t finalTime;

    double x;
    double y;
    double z;

    printf("Accelerometer\n");

    /*
     * Specify the registers that correspond to the accelerometer channel
     * that needs to be accessed.
     */
    Accl.xval = ACCXVAL;
    Accl.yval = ACCYVAL;
    Accl.zval = ACCZVAL;
    Accl.scale_wght = ACCSCALEWGHT;

    /*
     * Open the myRIO NiFpga Session.
     * This function MUST be called before all other functions. After this call
     * is complete the myRIO target will be ready to be used.
     */
    status = MyRio_Open();
    if (MyRio_IsNotSuccess(status))
    {
        return status;
    }

    /*
     * Read the scaling factors, initializing the struct.
     */
    Accel_Scaling(&Accl);

    /*
     * Normally, the main function runs a long running or infinite loop.
     * Read the accelerometer output for 60 seconds.
     */
    time(&currentTime);
    finalTime = currentTime + LoopDuration;
    printTime = currentTime;
    while (currentTime < finalTime)
    {
        time(&currentTime);

        /* Don't print every loop iteration. */
        if (currentTime > printTime)
        {
            x = Accel_ReadX(&Accl);
            y = Accel_ReadY(&Accl);
            z = Accel_ReadZ(&Accl);
            printf("X: %g, Y: %g, Z: %g\n", x, y, z);

            printTime += LoopSteps;
        }
    }

    /*
     * Close the myRIO NiFpga Session.
     * This function MUST be called after all other functions.
     */
    status = MyRio_Close();

    /*
     * Returns 0 if successful.
     */
    return status;
}
