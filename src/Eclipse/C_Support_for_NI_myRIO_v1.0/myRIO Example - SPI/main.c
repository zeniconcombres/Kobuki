/*
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include "SPI.h"
#include "MyRio.h"

extern NiFpga_Session myrio_session;

#if !defined(LoopDuration)
#define LoopDuration    60  /* How long to monitor the signal, in seconds */
#endif


/**
 * Overview:
 * Demonstrates using the SPI. Writes a message to the SPI bus, prints any
 * returned bytes to the console.
 *
 * Instructions:
 * 1. Connect an SPI slave to the SPI pins on connector A.
 * 2. Run the program.
 *
 * Output:
 * The program writes the message "Hello World\n" for 60 s. Slave values are
 * written to the console every newline character.3
 *
 * Note:
 * The Eclipse project defines the preprocessor symbol for the NI myRIO-1900.
 * Change the preprocessor symbol to use this example with the NI myRIO-1950.
 */
int main(int argc, char **argv)
{
    NiFpga_Status status;

    MyRio_Spi spiA;

    uint8_t selectReg;

    time_t currentTime;
    time_t finalTime;

    char writeMessage[]  = "Hello World\n";
    uint8_t writePos;
    uint16_t readChar;
    char readMessage[100];
    uint8_t readPos;

    printf("SPI\n");

    /*
     * Initialize the SPI struct with registers from the FPGA personality.
     */
    spiA.cnfg = SPIACNFG;
    spiA.cnt = SPIACNT;
    spiA.dato = SPIADATO;
    spiA.dati = SPIADATI;
    spiA.go = SPIAGO;
    spiA.stat = SPIASTAT;

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
     * SPI connections are on pins shared with other onboard devices. To use
     * on a physical pin, select the SPI on the appropriate SELECT register.
     *
     * Read the value of the SYSSELECTA register.
     */
    status = NiFpga_ReadU8(myrio_session, SYSSELECTA, &selectReg);
    MyRio_ReturnValueIfNotSuccess(status, status,
            "Could not read from the SYSSELECTA register!")

    /*
     * Set bit1 and bit 2 of the SYSSELECTA register to enable SPI bidirectional
     * functionality. The functionality of these bits is specified in the
     * documentation.
     *
     * Don't need to previously clear the bits because both bits are set.
     */
    selectReg = selectReg | 3;

    /*
     * Write the updated value of the SYSSELECTA register.
     */
    status = NiFpga_WriteU8(myrio_session, SYSSELECTA, selectReg);
    MyRio_ReturnValueIfNotSuccess(status, status,
            "Could not write to the SYSSELECTA register!")

    /*
     * Configure the SPI as leading clock phase, low clock polarity, most
     * significant bit first, 8 bits, and with a clock divider of 8X.
     */
    Spi_Configure(&spiA, Spi_ClockPhase | Spi_ClockPolarity | Spi_DataOrder
                  | Spi_FrameLength | Spi_ClockDivider
                  , Spi_ClockPhaseLeading | Spi_ClockPolarityLow
                  | Spi_DataOrderMsbFirst | Spi_FrameSize8 | Spi_Clock8x);

    /*
     * Set the maximum counter value. The counter counts from 0 to 1000.
     *
     * The clock divider was previously set with a clock divider of 8X.
     * The counter increments at 40 MHz / 2 / 8 = 2.5 MHz and the counter
     * counts from 0 to 5624990000. The frequency of the SPI is 2.5 MHz / 62499
     * = 40 Hz.
     */
    Spi_CounterMaximum(&spiA, 62499U);

    /*
     * Normally, the main function runs a long running or infinite loop.
     * Transmit with the SPI for 60 seconds. Transmits the message every
     * interaction. The SPI speed limits the output data rate.
     */
    time(&currentTime);
    finalTime = currentTime + LoopDuration;
    writePos = 0;
    readPos = 0;
    while (currentTime < finalTime)
    {
        Spi_Transmit(&spiA, writeMessage[writePos], &readChar);

        /*
         * Increment the position in the message. Reset to the beginning after
         * reaching the end of the message. -1 to not write the null terminating
         * character.
         */
        ++writePos;
        if (writePos >= (sizeof writeMessage) - 1)
        {
            writePos = 0;
        }

        /*
         * If the character read is not 0, then add to the string. Otherwise,
         * interpret 0 as 'nothing was read'
         */
        if (readChar != 0)
        {
            readMessage[readPos] = (char) readChar;

            /*
             * If this was a newline character, then print out the string
             * otherwise increment the position for the next loop iteration.
             */
            if (readMessage[readPos] == '\n')
            {
                readMessage[readPos + 1] = '\0';
                printf("%s", readMessage);
                readPos = 0;
            }
            else if (readPos < (sizeof readMessage) - 1)
            {
                ++readPos;
            }
        }

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
