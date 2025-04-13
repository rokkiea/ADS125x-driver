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
     printf("Opening device %s with speed: %d, mode: %d, spiBPW: %d\n", spidev, speed, mode, spiBPW);
 
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
 
 int ads1256Setup(ads125x_dev *dev, int spiChannel, int spiPort)
 {
     struct wiringPiNodeStruct *node;
     int fd;
 
     fd = SPISetup(0, 0, dev->spi_speed, dev->spi_bit_p_word, dev->spi_mode);
     return fd;
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
 
     spiTxData[0] = ADS1256_CMD_WREG | ADS1256_REG_ADDR_MUX;
     spiTxData[1] = 0x00;
     spiTxData[2] = psel | nsel;
 
     spi.tx_buf = (unsigned long)spiTxData;
     spi.rx_buf = 0;
     spi.len = 3;
     spi.delay_usecs = 0;
     spi.speed_hz = dev->spi_speed;
     spi.bits_per_word = dev->spi_bit_p_word;
     spi.cs_change = 0;
 
     if ((ret = ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi)) < 0)
         FailurePrint("Set Data-Rate error: %s\n", strerror(errno));
     return;
 }
 
 /**
  * ads125xSetDRATE - set ads125x_dev A/D data rate
 * @dev: The ads125x dev info struct pointer.
  * @dr: Target data rates, see ADS1256_DR_* and the datasheet.
  */
 void ads125xSetDRATE(ads125x_dev *dev, const uint8_t dr)
 {
     uint8_t spiTxData[3] = {0};
     int ret;
     struct spi_ioc_transfer spi;
 
     memset(&spi, 0, sizeof(spi));
 
     spiTxData[0] = ADS1256_CMD_WREG | ADS1256_REG_ADDR_DRATE;
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
  * Commands are macro-defined with the `ADS1256_CMD_` prefix.
 *
 * **WARN**: DO NOT USE THIS FUNCTION SEND RDATA/RRED/WREG COMMAND.
  */
 void ads125xSendCMD(ads125x_dev *dev, const uint8_t cmd)
 {
     uint8_t spiTxData = cmd;
     int ret;
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
 
     if (len > 0x0A)
     {
         fprintf(stderr, "%s", "ADS125x err: RREG data too long. (MAX 10 Bytes)\n");
         return;
     }
     memset(&spi, 0, sizeof(spi));
     memset(spiTxData, 0x0, len + 3);
 
     spiTxData[0] = ADS1256_CMD_RREG | (regaddr & 0x0F);
     spiTxData[1] = len & 0x0F;
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
     spi[1].len = len;
     spi[1].delay_usecs = 10;
     spi[1].speed_hz = dev->spi_speed;
     spi[1].bits_per_word = dev->spi_bit_p_word;
     spi[1].cs_change = 0;
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
 
     spiTxData[0] = ADS1256_CMD_WREG | (regaddr & 0x0F);
     spiTxData[1] = len & 0x0F;
     memcpy(spiTxData + 2, data, len);
 
     spi.tx_buf = (unsigned long)spiTxData;
     spi.rx_buf = 0;
     spi.len = len + 2;
     spi.delay_usecs = 0;
     spi.speed_hz = dev->spi_speed;
     spi.bits_per_word = dev->spi_bit_p_word;
     spi.cs_change = 0;
 
     if (ioctl(dev->fd, SPI_IOC_MESSAGE(1), &spi) < 0)
     {
         FailurePrint("WREG err: %s\n", strerror(errno));
         free(spiTxData);
     }
     free(spiTxData);
     return;
 }
 