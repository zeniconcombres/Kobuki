/*
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include "DIO.h"
#include "MyRio.h"

#if !defined(LoopDuration)
#define LoopDuration    60  /* How long to output the signal, in seconds */
#endif


/**
 * Overview:
 * Demonstrates using the digital input and output (DIO). Reads initial values
 * of two digital input channels from connector A and writes the Boolean AND of
 * the read values on connector B. Prints the values to the console.
 *
 * Instructions:
 * 1. Connect a DC voltage input (either 0 V or 5 V) to DIO 0 on connector A.
 * 2. Connect a DC voltage input (either 0 V or 5 V) to DIO 7 on connector A.
 * 3. Connect a voltmeter to DIO 7 on connector B.
 * 4. Run this program.
 *
 * Output:
 * The program reads the initial voltage on DI0 0 and DIO 7 on connector A, and
 * writes the Boolean AND on DIO 7 on connector B. for 60 s. The output is
 * maintained for 60 s. Input voltages and the output are written to the
 * console.
 *
 * Note:
 * The Eclipse project defines the preprocessor symbol for the NI myRIO-1900.
 * Change the preprocessor symbol to use this example with the NI myRIO-1950.
 */
int main(int argc, char **argv)
{
    NiFpga_Status status;
    NiFpga_Bool dio_A0;
    NiFpga_Bool dio_A7;
    NiFpga_Bool dio_B7;

    MyRio_Dio A0;
    MyRio_Dio A7;
    MyRio_Dio B7;

    time_t currentTime;
    time_t finalTime;

    printf("DigitalInputOutput\n");

    /*
     * Specify the registers that correspond to the DIO channel
     * that needs to be accessed.
     */
    A0.dir = DIOA_70DIR;
    A0.out = DIOA_70OUT;
    A0.in = DIOA_70IN;
    A0.bit = 0;

    A7.dir = DIOA_70DIR;
    A7.out = DIOA_70OUT;
    A7.in = DIOA_70IN;
    A7.bit = 7;

    B7.dir = DIOB_70DIR;
    B7.out = DIOB_70OUT;
    B7.in = DIOB_70IN;
    B7.bit = 7;

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
     * Read from DIO channels A/DIO0 and A/DIO7.
     * Each channel is on the same 8-channel bank on Connector A.
     */
    dio_A0 = Dio_ReadBit(&A0);
    dio_A7 = Dio_ReadBit(&A7);

    /*
     * Print out the logic level of each channel.
     */
    printf("A/DIO0 = %d\n", dio_A0);
    printf("A/DIO7 = %d\n", dio_A7);

    /*
     * If the A/DIO0 and B/DIO10 are both 1 then B/DIO7 should be 1.
     */
    dio_B7 = dio_A0 & dio_A7;

    /*
     * Print out the desired logic level of B/DIO7.
     */
    printf("B/DIO7 = %d\n", dio_B7);

    /*
     * Write to channel B/DIO7 to set it to the desired value.
     */
    Dio_WriteBit(&B7, dio_B7);

    /*
     * Normally, the main function runs a long running or infinite loop.
     * Keep the program running so that you can measure the output from B7 using
     * an external instrument.
     */
    time(&currentTime);
    finalTime = currentTime + LoopDuration;
    while (currentTime < finalTime)
    {
        time(&currentTime);
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
