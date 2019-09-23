/*
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include "AIO.h"
#include "MyRio.h"

#if !defined(LoopDuration)
#define LoopDuration    60  /* How long to output the signal, in seconds */
#endif


/**
 * Overview:
 * Demonstrates using the analog input and output (AIO). Reads initial values
 * of two analog input channels from connector A and writes the sum of the read
 * values on connector B. Prints the values to the console.
 *
 * Instructions:
 * 1. Connect a DC voltage input (between 0 V and 5 V) to AI0 on connector A.
 * 2. Connect a DC voltage input (between 0 V and 5 V) to AI3 on connector A.
 * 3. Connect a voltmeter to AO0 on connector B.
 * 4. Run this program.
 *
 * Output:
 * The program reads the initial voltage on AI0 and AI3 on connector A, and
 * writes the sum to AO0 on connector B. The output is maintained for 60 s.
 * Input voltages and the sum are written to the console.
 *
 * Note:
 * The Eclipse project defines the preprocessor symbol for the NI myRIO-1900.
 * Change the preprocessor symbol to use this example with the NI myRIO-1950.
 */
int main(int argc, char **argv)
{
    NiFpga_Status status;
    double ai_A0;
    double ai_A3;
    double ao_B0;

    MyRio_Aio A0;
    MyRio_Aio A3;
    MyRio_Aio B0;
    MyRio_Aio_Rdy Aio_Ready;

    time_t currentTime;
    time_t finalTime;

    printf("AnalogInputOutput\n");

    /*
     * Specify the registers that correspond to the AIO channel
     * that needs to be accessed.
     */
    A0.val = AIA_0VAL;
    A0.wght = AIA_0WGHT;
    A0.ofst = AIA_0OFST;
    A0.is_signed = NiFpga_False;

    A3.val = AIA_3VAL;
    A3.wght = AIA_3WGHT;
    A3.ofst = AIA_3OFST;
    A3.is_signed = NiFpga_False;

    B0.val = AOB_0VAL;
    B0.wght = AOB_0WGHT;
    B0.ofst = AOB_0OFST;
    B0.set = AOSYSGO;
    B0.is_signed = NiFpga_True;

    Aio_Ready.ai_rdy = SYSAIRDY;
    Aio_Ready.ai_scale_rdy = SYSAI_SCALERDY;
    Aio_Ready.ao_scale_rdy = SYSAO_SCALERDY;

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
     * Read the scaling factors, initializing the struct
     */
    Aio_Scaling(&A0);
    Aio_Scaling(&A3);
    Aio_Scaling(&B0);

    /*
     * Read from AI channels A/AI0 and A/AI3.
     * Each channel is on Connector A.
     */
    ai_A0 = Aio_Read(&A0);
    ai_A3 = Aio_Read(&A3);

    /*
     * Print out the logic level of each channel.
     */
    printf("A/AI0 = %f\n", ai_A0);
    printf("A/AI3 = %f\n", ai_A3);

    /*
     * Calculate the difference between the channels
     */
    ao_B0 = ai_A0 - ai_A3;

    /*
     * Print out the desired logic level of B/AO0.
     */
    printf("B/AO0 = %f\n", ao_B0);

    /*
     * Write to channel B/AO0 to set it to the desired value.
     */
    Aio_Write(&B0, ao_B0);

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
