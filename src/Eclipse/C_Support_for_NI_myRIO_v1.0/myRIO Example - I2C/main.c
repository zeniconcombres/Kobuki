/*
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>
#include "I2C.h"
#include "MyRio.h"

/*
 * Declare the myRIO NiFpga_Session so that it can be used by any function in
 * this file. The variable is actually defined in myRIO.c.
 */
extern NiFpga_Session myrio_session;


/**
 * Overview:
 * Demonstrates using the I2C. Read the temperature from a connected TMP102
 * digital temperature sensor and writes the response to the console.
 *
 * Instructions:
 * 1. Connect a TMP102 digital temperature sensor to the I2C pins on
 *    connector A.
 * 2. Run the program.
 *
 * Output:
 * The program writes the read temperature to the console.
 *
 * Note:
 * The Eclipse project defines the preprocessor symbol for the NI myRIO-1900.
 * Change the preprocessor symbol to use this example with the NI myRIO-1950.
 */
int main(int argc, char **argv)
{
    NiFpga_Status status;
    uint8_t data[2] = {0x00, 0x80};
    int index;
    uint8_t slaveWriteAddress = 0x48;
    uint8_t slaveReadAddress = 0x48;
    MyRio_I2c i2cA;

    uint8_t selectReg;

    printf("I2C\n");

    /*
     * Initialize the I2C struct with registers from the FPGA personality.
     */
    i2cA.addr = I2CAADDR;
    i2cA.cnfg = I2CACNFG;
    i2cA.cntl = I2CACNTL;
    i2cA.cntr = I2CACNTR;
    i2cA.dati = I2CADATI;
    i2cA.dato = I2CADATO;
    i2cA.go = I2CAGO;
    i2cA.stat = I2CASTAT;

    /*
     * Open the myRIO NiFpga Session.
     * This function MUST be called before all other functions. After this call
     * is complete the FPGA will be loaded and the myRIO target will be ready
     * to be used.
     */
    status = MyRio_Open();

    if (MyRio_IsNotSuccess(status))
    {
        return status;
    }

    /*
     * Enable the I2C functionality on Connector A
     *
     * Read the value of the SELECTA register.
     */
    status = NiFpga_ReadU8(myrio_session, SYSSELECTA, &selectReg);

    MyRio_ReturnValueIfNotSuccess(status, status,
                "Could not read from the SYSSELECTA register!");

    /*
     * Set bit7 of the SELECT register to enable the I2C functionality. The
     * functionality of this bit is specified in the documentation.
     */
    selectReg = selectReg | (1 << 7);

    /*
     * Write the updated value of the SELECT register.
     */
    status = NiFpga_WriteU8(myrio_session, SYSSELECTA, selectReg);

    MyRio_ReturnValueIfNotSuccess(status, status,
                    "Could not write to the SYSSELECTA register!");

    /*
     * Set the speed of the I2C block.
     *
     * Standard mode (100 kbps) = 187.
     * Fast mode (400 kbps) = 51.
     *
     * These values are calculated using the formula:
     *   f_SCL = f_clk / (2 * CNTR) - 4
     *
     * where:
     *   f_SCL = the desired frequency of the I2C transmission
     *   f_clk = the frequency of the myRIO FPGA clock (40 Mhz default)
     *
     * This formula and its rationale can be found in the documentation.
     */
    I2c_Counter(&i2cA, 187);

     /*
      * Enable the I2C block.
      */
    I2c_Configure(&i2cA, I2c_Enabled);

    /*
     * Send 2 bytes to the slave device.
     */
    I2c_Write(&i2cA, slaveWriteAddress, data, 2);

    /*
     * Read 10 bytes from the slave device.
     */
    I2c_Read(&i2cA, slaveReadAddress, data, 2);

    /*
     * Print the data received from the slave.
     */
    printf("Received data:");
    for (index = 0; index < 2; index++)
    {
        printf(" %.2X ", data[index]);
    }
    printf("\n");

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
