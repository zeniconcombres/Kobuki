/*
 * Example for performing SPI
 *
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>

/*
 * Include the myRIO header file.
 * The target type must be defined in your project, as a stand-alone #define,
 * or when calling the compiler from the command-line.
 */
#include "MyRio.h"
#include "SPI.h"


/*
 * Declare the myRIO NiFpga_Session so that it can be used by any function in
 * this file. The variable is actually defined in myRIO.c.
 *
 * This removes the need to pass the myrio_session around to every function and
 * only has to be declared when it is being used.
 */
extern NiFpga_Session myrio_session;


/**
 * Flags for the SPI status register.
 */
typedef enum
{
    Spi_Busy = 0x1,   /* 0b00000001 */
} Spi_StatusMask;


/**
 * Sets options for the SPI configuration register.
 *
 * @param[in]  channel  A struct containing the registers on the SPI channel to
 *                      modify.
 * @param[in]  mask     Array of flags that indicate which of the configure
 *                      settings are valid.
 * @param[in]  settings Array of flags that indicate the configuration settings.
 */
void Spi_Configure(MyRio_Spi* channel, Spi_ConfigureMask mask,
                   Spi_ConfigureSettings settings)
{
    NiFpga_Status status;
    uint16_t cnfgValue;

    /*
     * Get the current value of the configure register.
     *
     * The returned NiFpga_Status value is stored for error checking.
     */
    status = NiFpga_ReadU16(myrio_session, channel->cnfg, &cnfgValue);

    /*
     * Check if there was an error reading from the SPI registers.
     *
     * If there was an error then the rest of the function cannot complete
     * correctly so print an error message to stdout and return from the
     * function early.
     */
    MyRio_ReturnIfNotSuccess(status,
            "Could not read from the SPI configure registers!")

    /*
     * Clear the value of the masked bits in the configure register. This is
     * done so that the correct value can be set later on.
     */
    cnfgValue = cnfgValue & ~mask;

    /*
     * Set the value of the settings bits in the configure register. If the
     * value to set is 0 this operation would not work unless the bit was
     * previously cleared.
     */
    cnfgValue = cnfgValue | settings;

    /*
     * Write the new value of the configure register to the device.
     */
    status = NiFpga_WriteU16(myrio_session, channel->cnfg, cnfgValue);

    /*
     * Check if there was an error writing to SPI configure registers.
     *
     * If there was an error then print an error message to stdout.
     */
    MyRio_ReturnIfNotSuccess(status,
            "Could not write to the SPI configure registers!")
}


/**
 * Sets the maximum counter value for the SPI. This, along with the clock
 * divider value set by Spi_Configure sets the speed of the SPI transmission.
 *
 * SPI frequency  = f_SPI / divisor /  2 * (maximum counter + 1)
 *
 * where f_SPI = 80 MHz.
 *
 * @param[in]  channel  A struct containing the registers of the SPI channel to
 *                      modify.
 * @param[in]  counterMax  The maximum counter value.
 */
void Spi_CounterMaximum(MyRio_Spi* channel, uint16_t counterMax)
{
    NiFpga_Status status;

    /*
     * Write the new value of the clock select register to the device.
     */
    status = NiFpga_WriteU16(myrio_session, channel->cnt, counterMax);

    /*
     * Check if there was an error writing to SPI counter registers.
     *
     * If there was an error then print an error message to stdout.
     */
    MyRio_ReturnIfNotSuccess(status,
            "Could not write to the SPI maximum register!")
}


/**
 * Transmits data via SPI.
 */
void Spi_Transmit(MyRio_Spi* channel, uint16_t dataOut, uint16_t* dataIn)
{
    NiFpga_Status status;
    uint8_t spiStatus;

    /*
     * Set the value of the data to output.
     *
     * The returned NiFpga_Status value is stored for error checking.
     */
    status = NiFpga_WriteU16(myrio_session, channel->dato, dataOut);

    /*
     * Start the data transmission.
     *
     * NiFpga_MergeStatus is used to propagate any errors from previous
     * function calls. Errors are not anticipated so error checking is not done
     * after every NiFpga function call but only at specific points.
     */
    NiFpga_MergeStatus(&status,
        NiFpga_WriteBool(myrio_session, channel->go, NiFpga_True));

    /*
     * Check if there was an error writing to the SPI registers.
     *
     * If there was an error then the rest of the function cannot complete
     * correctly so print an error message to stdout and return from the
     * function early.
     */
    MyRio_ReturnIfNotSuccess(status, "Could not write to the SPI registers!")

    /*
     * Wait for the status register to be set, which indicates that the transfer
     * completed.
     */
    do
    {
        /*
         * Read the status register to check if it inverted.
         *
         * The returned NiFpga_Status value is stored for error checking.
         */
        status = NiFpga_ReadU8(myrio_session, channel->stat, &spiStatus);


    } while ((spiStatus & Spi_Busy) && !MyRio_IsNotSuccess(status));

    /*
     * Check if there was an error reading from the SPI registers.
     *
     * If there was an error then the rest of the function cannot complete
     * correctly so print an error message to stdout and return from the
     * function early.
     */
    MyRio_ReturnIfNotSuccess(status,
            "Could not read from the SPI stat register!")

    /*
     * NULL indicates don't care about reading data.
     */
    if (dataIn != NULL)
    {
        /*
         * Get the value of the data input register.
         *
         * The returned NiFpga_Status value is stored for error checking.
         */
        status = NiFpga_ReadU16(myrio_session, channel->dati, dataIn);

        /*
         * Check if there was an error reading from the SPI registers.
         *
         * If there was an error then print an error message to stdout.
         */
        MyRio_ReturnIfNotSuccess(status,
                "Could not read from the SPI dati register!")
    }
}
