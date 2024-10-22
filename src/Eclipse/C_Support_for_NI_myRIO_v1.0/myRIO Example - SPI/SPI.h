/*
 * spi.h
 *
 * Copyright (c) 2013,
 * National Instruments Corporation.
 * All rights reserved.
 */

#ifndef SPI_h_
#define SPI_h_

#include "MyRio.h"

#if NiFpga_Cpp
extern "C" {
#endif


/**
 * Flags that indicate which of the SPI configure functions are set.
 */
typedef enum
{
    Spi_ClockPhase = 0x02,      /**< The clock phase option is set */
    Spi_ClockPolarity = 0x04,   /**< The clock polarity option is set */
    Spi_DataOrder = 0x08,       /**< The data order option is set */
    Spi_FrameLength = 0xF0,     /**< The frame length option is set */
    Spi_ClockDivider = 0xC000   /**< The clock divider option is set */
} Spi_ConfigureMask;


/**
 * SPI configure register options.
 */
typedef enum
{
    Spi_ClockPhaseLeading = 0x00,       /**< Setup trailing, sample leading */
    Spi_ClockPhaseTrailing = 0x02,      /**< Setup leading, sample trailing */
    Spi_ClockPolarityLow = 0x00,        /**< SCLK is low when idle */
    Spi_ClockPolarityHigh = 0x04,       /**< SCLK is high when idle */
    Spi_DataOrderMsbFirst = 0x00,       /**< Most significant bit first */
    Spi_DataOrderLsbFirst = 0x08,       /**< Least significant bit first */
    Spi_FrameSize1 = 0x00,              /**< 1 bit frame size */
    Spi_FrameSize2 = 0x10,              /**< 2 bit frame size */
    Spi_FrameSize3 = 0x20,              /**< 3 bit frame size */
    Spi_FrameSize4 = 0x30,              /**< 4 bit frame size */
    Spi_FrameSize5 = 0x40,              /**< 5 bit frame size */
    Spi_FrameSize6 = 0x50,              /**< 6 bit frame size */
    Spi_FrameSize7 = 0x60,              /**< 7 bit frame size */
    Spi_FrameSize8 = 0x70,              /**< 8 bit frame size */
    Spi_FrameSize9 = 0x80,              /**< 9 bit frame size */
    Spi_FrameSize10 = 0x90,             /**< 10 bit frame size */
    Spi_FrameSize11 = 0xA0,             /**< 11 bit frame size */
    Spi_FrameSize12 = 0xB0,             /**< 12 bit frame size */
    Spi_FrameSize13 = 0xC0,             /**< 13 bit frame size */
    Spi_FrameSize14 = 0xD0,             /**< 14 bit frame size */
    Spi_FrameSize15 = 0xE0,             /**< 15 bit frame size */
    Spi_FrameSize16 = 0xF0,             /**< 16 bit frame size */
    Spi_Clock1x = 0x0000,               /**< 1x (f_SPI) */
    Spi_Clock2x = 0x4000,               /**< 2x (f_SPI / 2) */
    Spi_Clock4x = 0x8000,               /**< 4x (f_SPI / 4) */
    Spi_Clock8x = 0xC000                /**< 8x (f_SPI / 8) */
} Spi_ConfigureSettings;


/**
 * Registers for a particular SPI.
 */
typedef struct
{
    uint32_t cnfg;  /**< SPI configuration register */
    uint32_t cnt;   /**< Maximum value of the counter register */
    uint32_t dato;  /**< Data to be transferred to the slave device */
    uint32_t dati;  /**< Data transferred from the slave device */
    uint32_t go;    /**< Signals data is available (start transfer) */
    uint32_t stat;  /**< Status of transfer */
} MyRio_Spi;


/**
 * Sets the SPI configuration options.
 */
void Spi_Configure(MyRio_Spi* channel, Spi_ConfigureMask mask,
                   Spi_ConfigureSettings settings);


/**
 * Sets the maximum value of the SPI counter.
 */
void Spi_CounterMaximum(MyRio_Spi* channel, uint16_t counterMax);


/**
 * Transmits data.
 */
void Spi_Transmit(MyRio_Spi* channel, uint16_t dataOut, uint16_t* dataIn);

#if NiFpga_Cpp
}
#endif

#endif /* SPI_h_ */
