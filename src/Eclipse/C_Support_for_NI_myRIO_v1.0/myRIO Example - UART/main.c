/*
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#include <stdio.h>
#include <time.h>
#include "UART.h"

#define BaudRate 9600
#define DataBit 8


/**
 * Overview:
 * Demonstrates using the UART. Writes a character to the UART bus, prints any
 * returned character to the console.
 *
 * Instructions:
 * 1. Connect a UART device that can receive and/or send character data to the
 *    UART pins on connector A.
 * 2. Run the program.
 *
 * Output:
 * The program writes the character "H" to the console. Returned character is
 * written to the console if the attached UART device writes to the bus.
 *
 * Note:
 * The Eclipse project defines the preprocessor symbol for the NI myRIO-1900.
 * Change the preprocessor symbol to use this example with the NI myRIO-1950.
 */
int main(int argc, char **argv)
{
    MyRio_Uart uart;

    uint8_t writeData = 'H';
    uint8_t readData = 0;
    int32_t status = 0;

    /*
     * Initialize the UART port structure.
     */
    uart.name = "ASRL1::INSTR";
    uart.defaultRM = 0;
    uart.session = 0;

    /*
     * Opens a UART session.
     */
    status = Uart_Open(&uart, BaudRate, DataBit,
                       Uart_StopBits1_0, Uart_ParityNone);
    if (status < VI_SUCCESS)
    {
        return status;
    }

    /*
     * Writes data to a UART port.
     */
    status = Uart_Write(&uart, &writeData, 1);
    if (status < VI_SUCCESS)
    {
        return status;
    }

    /*
     * Reads data from a UART port.
     */
    status = Uart_Read(&uart, &readData, 1);
    if (status < VI_SUCCESS)
    {
        return status;
    }

    printf("The received data is %d", readData);
    /*
     * Clears UART receive buffer.
     */
    status = Uart_Clear(&uart);
    if (status < VI_SUCCESS)
    {
        return status;
    }

    /*
     * Closes the UART port.
     */
    status = Uart_Close(&uart);
    if (status < VI_SUCCESS)
    {
        return status;
    }

    return status;
}
