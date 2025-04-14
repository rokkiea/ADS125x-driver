/**
 * libads1256.c - TI ads125x_dev driver library
 *
 *	Extend wiringPi with the ads125x_dev SPI 24-Bit ADC
 *	Copyright (c) 2025, Guo Ruijing (rokkiea)
 *
 * This program has only been tested on the Orange Pi 5 Pro and is not
 * guaranteed to function correctly on any other boards.
 *
 ***********************************************************************
 * This file exists as a supplement to WiringPi.
 *  https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ***********************************************************************
 * For details of ADS1256, see:
 *  TI ADS1256: https://www.ti.com/product/ADS1256
 *  Datasheet: https://www.ti.com/lit/gpn/ads1256
 */

#include <byteswap.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
// #include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

#include "libads1256reg.h"
#include "ads1256.h"
#include "libads1256.h"

int FailurePrint(const char *message, ...)
{
    va_list argp;
    char buffer[1024];

    va_start(argp, message);
    vsnprintf(buffer, 1023, message, argp);
    va_end(argp);

    fprintf(stderr, "%s", buffer);
    exit(EXIT_FAILURE);

    return 0;
}

/**
 * convert_to_signed_24bit - Convert 24-bit data to signed int
 * @result: A unsigned char data arrays.
 */
int32_t convert_to_signed_24bit(const unsigned char *result)
{
    int32_t value = (result[0] << 16) | (result[1] << 8) | result[2];
    if (value & 0x800000)
    {
        value |= 0xFF000000;
    }
    return value;
}

/**
 * ads125xGetGPIOLine - Get gpio line struct pointer
 * @chip: Target GPIO chip string.
 * @line: Target GPIO line number.
 * @cp: Target GPIO chip pointer
 * @lp: Target GPIO line pointer
 *
 * @return: return 0 is open chip and line successful.
 *          return 1 is open chip failed, return 2 is open line failed.
 */
int ads125xGetGPIOLine(char *chip, int line, struct gpiod_chip **cp, struct gpiod_line **lp)
{
    struct gpiod_chip *c;
    struct gpiod_line *l;

    if (!(c = gpiod_chip_open_by_name(chip)))
    {
        // perror("Cannot open gpio");
        fprintf(stderr, "Cannot open %s .", chip);
        return 1;
    }
    *cp = c;
    if (!(l = gpiod_chip_get_line(c, line)))
    {
        fprintf(stderr, "Cannot open %s line %d .\n", chip, line);
        gpiod_chip_close(c);
        return 2;
    }
    *lp = l;
    return 0;
}

/**
 * ads125xOpenDRDY - Open ADS1256 DRDY pin
 * @dev: The ads125x dev info struct pointer.
 * @chip: Target GPIO chip string.
 * @line: Target GPIO line number.
 *
 * @return: 0 is open DRDY successful,
 *          1 is open DRDY GPIO error,
 *          2 is set DRDY direction error.
 */
int ads125xOpenDRDY(ads125x_dev *dev, char *chip, int line)
{
    if (ads125xGetGPIOLine(chip, line, &(dev->pin_DRDY_chip), &(dev->pin_DRDY_line)))
    {
        fprintf(stderr, "Cannot open ADS1256 DRDY.\n");
        return 1;
    }
    if ((gpiod_line_request_input(dev->pin_DRDY_line, "ads125x-drdy")) < 0)
    {
        fprintf(stderr, "Cannot set DRDY to input mode.\n");
        return 2;
    }
    return 0;
}

/**
 * ads125xOpenPDWN - Open ADS1256 PDWN pin
 * @dev: The ads125x dev info struct pointer.
 * @chip: Target GPIO chip string.
 * @line: Target GPIO line number.
 * @init_status: Init PDWN status, 0 is low, 1 is high.
 *
 * @return: 0 is open DRDY successful,
 *          1 is open DRDY GPIO error,
 *          2 is set DRDY direction error.
 *          3 invalid init_status, only 0 or 1.
 */
int ads125xOpenPDWN(ads125x_dev *dev, char *chip, int line, uint8_t init_status)
{
    if (init_status & 0xFE)
    {
        fprintf(stderr, "Invalid init_status %d.\n", init_status);
        return 3;
    }
    if (ads125xGetGPIOLine(chip, line, &(dev->pin_PDWN_chip), &(dev->pin_PDWN_line)))
    {
        fprintf(stderr, "Cannot open ADS1256 PDWN.\n");
        return 1;
    }
    if ((gpiod_line_request_output(dev->pin_PDWN_line, "ads125x-pdwn", init_status)) < 0)
    {
        fprintf(stderr, "Cannot set DRDY to output mode.\n");
        return 2;
    }
    return 0;
}

/**
 * ads125xCloseDRDY - Close ADS1256 DRDY GPIO chip and line
 */
void ads125xCloseDRDY(ads125x_dev *dev)
{
    // gpiod_line_release(dev->pin_DRDY_line);
    gpiod_line_close_chip(dev->pin_DRDY_line);
    dev->pin_DRDY_line = NULL;
    dev->pin_DRDY_chip = NULL;
    return;
}

/**
 * ads125xClosePDWN - Close ADS1256 DRDY GPIO chip and line
 */
void ads125xClosePDWN(ads125x_dev *dev)
{
    // gpiod_line_release(dev->pin_DRDY_line);
    gpiod_line_close_chip(dev->pin_PDWN_line);
    dev->pin_PDWN_line = NULL;
    dev->pin_PDWN_chip = NULL;
    return;
}

/**
 * ads125xwaitDRDY - Wating ADS1256 DRDY to low
 */
void ads125xwaitDRDY(struct gpiod_line *line)
{
    while (gpiod_line_get_value(line))
        ;
    return;
}
/**
 * SPISetup - Set up a spi device
 * @channel: The bus to which the SPI device belongs.
 * @port: The device (chip select pin) number connected to this SPI bus.
 * @speed: Speed of the SPI device.
 * @mode: Clock Phase and Polarity
 */
int SPISetup(const int channel, const int port, const int speed, const int spiBPW, const int mode)
{
    int fd;
    static char spidev[14];

    sprintf(spidev, "/dev/spidev%i.%i", channel, port);
    fprintf(stdout, "Opening device %s with speed: %d, mode: %d, spiBPW: %d\n", spidev, speed, mode, spiBPW);

    // Get SPI dev descriptors
    if ((fd = open(spidev, O_RDWR)) < 0)
        return FailurePrint("SPI Mode Change failure: %s\n", strerror(errno));

    // Set SPI parameters.
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0)
        return FailurePrint("SPI Mode Change failure: %s\n", strerror(errno));
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBPW) < 0)
        return FailurePrint("SPI Mode Change failure: %s\n", strerror(errno));
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
        return FailurePrint("SPI Mode Change failure: %s\n", strerror(errno));

    return fd;
}

int ads125xSetup(ads125x_dev *dev, int spiChannel, int spiPort)
{
    struct wiringPiNodeStruct *node;
    int fd;

    fd = SPISetup(0, 0, dev->spi_speed, dev->spi_bit_p_word, dev->spi_mode);
    return fd;
}

int SPIRelease(const int fd)
{
    if (!close(fd))
    {
        fprintf(stdout, "Close SPI descriptors success.\n");
        return 0;
    }
    return 1;
}

/**
 * ads125xSetMUX:
 * @dev: The ads125x dev info struct pointer.
 * @psel: Postive Input Channel (AIN_P) select.
 * @nsel: Negative Input Channel (AIN_N) select.
 */
void ads125xSetMUX(ads125x_dev *dev, const uint8_t psel, const uint8_t nsel)
{
    uint8_t spiTxData[3] = {0};
    int ret;
    struct spi_ioc_transfer spi;

    memset(&spi, 0, sizeof(spi));

    spiTxData[0] = ADS125x_CMD_WREG | ADS125x_REG_ADDR_MUX;
    spiTxData[1] = 0x00;
    spiTxData[2] = psel | nsel;

    spi.tx_buf = (unsigned long)spiTxData;
    spi.rx_buf = 0;
    spi.len = 3;
    spi.delay_usecs = 0;
    spi.speed_hz = dev->spi_speed;
    spi.bits_per_word = dev->spi_bit_p_word;
    spi.cs_change = 0;

    while (gpiod_line_get_value(dev->pin_DRDY_line))
        ;
    if ((ret = ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi)) < 0)
        FailurePrint("Set Data-Rate error: %s\n", strerror(errno));
    return;
}

/**
 * ads125xSetDRATE - set ads125x_dev A/D data rate
 * @dev: The ads125x dev info struct pointer.
 * @dr: Target data rates, see ADS125x_DR_* and the datasheet.
 */
void ads125xSetDRATE(ads125x_dev *dev, const uint8_t dr)
{
    uint8_t spiTxData[3] = {0};
    int ret;
    struct spi_ioc_transfer spi;

    memset(&spi, 0, sizeof(spi));

    spiTxData[0] = ADS125x_CMD_WREG | ADS125x_REG_ADDR_DRATE;
    spiTxData[1] = 0x00;
    // spiTxData[2] = dr;
    spiTxData[2] = dr;

    spi.tx_buf = (unsigned long)spiTxData;
    spi.rx_buf = 0;
    spi.len = 3;
    spi.delay_usecs = 0;
    spi.speed_hz = dev->spi_speed;
    spi.bits_per_word = dev->spi_bit_p_word;
    spi.cs_change = 0;

    while (gpiod_line_get_value(dev->pin_DRDY_line))
        ;
    if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
        FailurePrint("Set Data-Rate error: %s\n", strerror(errno));
    return;
}

/**
 * ads125xSendCMD - Send a command to ADS1256
 * @dev: The ads125x dev info struct pointer.
 * @cmd: A command. see ads1256reg.h .
 *
 * This function is intended for sending single-byte commands only,
 * facilitating operations such as wake-up, calibration, and reset.
 * Commands are macro-defined with the `ADS125x_CMD_` prefix.
 *
 * **WARN**: DO NOT USE THIS FUNCTION SEND RDATA/RRED/WREG COMMAND.
 */
void ads125xSendCMD(ads125x_dev *dev, const uint8_t cmd)
{
    uint8_t spiTxData = cmd;
    int ret, v;
    struct spi_ioc_transfer spi;

    memset(&spi, 0, sizeof(spi));

    spiTxData = cmd;

    spi.tx_buf = (unsigned long)&spiTxData;
    spi.rx_buf = 0;
    spi.len = 1;
    spi.delay_usecs = 0;
    spi.speed_hz = dev->spi_speed;
    spi.bits_per_word = dev->spi_bit_p_word;
    spi.cs_change = 0;

    while (gpiod_line_get_value(dev->pin_DRDY_line))
        ;
    if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
        FailurePrint("Send command error: %s\n", strerror(errno));
    return;
}

/**
 * ads125xRREG - read some data from registers
 * @dev: The ads125x dev info struct pointer.
 * @regaddr: The target read register address.
 * @data: Used to store the read data.
 * @len: Read data length, not need to -1
 */
void ads125xRREG(ads125x_dev *dev, const uint8_t regaddr, uint8_t *data, const uint8_t len)
{
    uint8_t *spiTxData = (uint8_t *)malloc(len + 3);
    int ret;
    struct spi_ioc_transfer spi[2];

    if (len > 0x0A || len < 1)
    {
        fprintf(stderr, "ADS125x err: invalid RREG data length %d. (MAX 10 Bytes)\n", len);
        return;
    }
    memset(&spi, 0, sizeof(spi));
    memset(spiTxData, 0x0, len + 3);

    spiTxData[0] = ADS125x_CMD_RREG | (regaddr & 0x0F);
    spiTxData[1] = (len - 1) & 0x0F;
    // Send RREG command
    spi[0].tx_buf = (unsigned long)spiTxData;
    spi[0].rx_buf = 0;
    spi[0].len = 2;
    spi[0].delay_usecs = 4;
    spi[0].speed_hz = dev->spi_speed;
    spi[0].bits_per_word = dev->spi_bit_p_word;
    spi[0].cs_change = 0;

    // Received data
    spi[1].tx_buf = (unsigned long)spiTxData + 2;
    spi[1].rx_buf = (unsigned long)data;
    spi[1].len = (len - 1);
    spi[1].delay_usecs = 0;
    spi[1].speed_hz = dev->spi_speed;
    spi[1].bits_per_word = dev->spi_bit_p_word;
    spi[1].cs_change = 0;

    while (gpiod_line_get_value(dev->pin_DRDY_line))
        ;
    if (ioctl(dev->fd, SPI_IOC_MESSAGE(2), &spi) < 0)
    {
        FailurePrint("WREG err: %s\n", strerror(errno));
        free(spiTxData);
    }
    free(spiTxData);
    return;
}

/**
 * ads125xWREG - Write some data to registers
 * @dev: The ads125x dev info struct pointer.
 * @regaddr: The target write register address.
 * @data: Used to store the data to be written.
 * @len: Write data length, not need to -1
 */
void ads125xWREG(ads125x_dev *dev, const uint8_t regaddr, uint8_t *data, const uint8_t len)
{
    uint8_t *spiTxData = (uint8_t *)malloc(len + 2);
    int ret;
    struct spi_ioc_transfer spi;

    if (len > 0x0A)
    {
        fprintf(stderr, "%s", "ADS125x err: WREG data too long. (MAX 64 Bytes)\n");
        return;
    }
    memset(&spi, 0, sizeof(spi));
    memset(spiTxData, 0x0, len + 2);

    spiTxData[0] = ADS125x_CMD_WREG | (regaddr & 0x0F);
    spiTxData[1] = len & 0x0F;
    memcpy(spiTxData + 2, data, len);

    spi.tx_buf = (unsigned long)spiTxData;
    spi.rx_buf = 0;
    spi.len = len + 2;
    spi.delay_usecs = 0;
    spi.speed_hz = dev->spi_speed;
    spi.bits_per_word = dev->spi_bit_p_word;
    spi.cs_change = 0;

    while (gpiod_line_get_value(dev->pin_DRDY_line))
        ;
    if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
    {
        FailurePrint("WREG err: %s\n", strerror(errno));
        free(spiTxData);
    }
    free(spiTxData);
    return;
}

/**
 * ads125xRDATA - ADS125x one-shot read data
 * @dev: The ads125x dev info struct pointer.
 * @data: Used to store the data to be written, please give a 3-Bytes space.
 */
void ads125xRDATA(ads125x_dev *dev, uint8_t *data)
{
    uint8_t spiTxData = ADS125x_CMD_RDATA;
    struct spi_ioc_transfer spi[2];

    memset(&spi, 0, sizeof(spi));
    // Send RDATA command
    spi[0].tx_buf = (unsigned long)&spiTxData;
    spi[0].rx_buf = 0;
    spi[0].len = 1;
    spi[0].delay_usecs = 4;
    spi[0].speed_hz = dev->spi_speed;
    spi[0].bits_per_word = dev->spi_bit_p_word;
    spi[0].cs_change = 0;
    // Received data
    spi[1].tx_buf = 0;
    spi[1].rx_buf = (unsigned long)data;
    spi[1].len = 3;
    spi[1].delay_usecs = 0;
    spi[1].speed_hz = dev->spi_speed;
    spi[1].bits_per_word = dev->spi_bit_p_word;
    spi[1].cs_change = 0;

    if (ioctl(dev->fd, SPI_IOC_MESSAGE(2), &spi) < 0)
        FailurePrint("RDATA error: %s\n", strerror(errno));
    return;
}

/**
 * ads125xRDATAC - ADS125x continuous read data
 * @dev: The ads125x dev info struct pointer.
 * @data: Used to store the data to be written, please give times*3 space.
 * @times: Read times
 */
void ads125xRDATAC(ads125x_dev *dev, uint8_t *data, int times)
{
    int i;
    // int g;
    struct spi_ioc_transfer spi;

    memset(&spi, 0, sizeof(spi));
    spi.tx_buf = 0;
    spi.rx_buf = (unsigned long)data;
    spi.len = 3;
    spi.delay_usecs = 0;
    spi.speed_hz = dev->spi_speed;
    spi.bits_per_word = dev->spi_bit_p_word;
    spi.cs_change = 0;

    ads125xSendCMD(dev, ADS125x_CMD_RDATAC);
    // ads125xwaitDRDY(dev->pin_DRDY_line);
    // if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
    //     FailurePrint("Send command error: %s\n", strerror(errno));
    for (i = 0; i < times; ++i)
    {
        spi.rx_buf = (unsigned long)(data + 3 * i);
        ads125xwaitDRDY(dev->pin_DRDY_line);
        // while ((g = gpiod_line_get_value(dev->pin_DRDY_line)))
        //     if (g == -1)
        //         exit(1);
        if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
            FailurePrint("RDATAC error: %s\n", strerror(errno));
        // printf("time: %d\n", i);
    }
    ads125xSendCMD(dev, ADS125x_CMD_SDATAC);
    return;
}

/**
 * ads125xSetPDWN - Set ADS1256 PDWN
 * @dev: The ads125x dev info struct pointer.
 * @status: PDWN status, 0 is low, 1 is high.
 *
 * @return: 1 is Invalid status.
 *          2 is set gpio line value failed.
 */
int ads125xSetPDWN(ads125x_dev *dev, uint8_t status)
{
    if (status & 0xFE)
    {
        fprintf(stderr, "Invalid status %d.\n", status);
        return 1;
    }
    // printf("a\n");
    return (gpiod_line_set_value(dev->pin_PDWN_line, status));
}