/**
 * libads1256.h - TI ADS1255/ADS1256 driver library
 *
 *	Driver for the ADS1256 SPI 24-Bit ADC
 *	Copyright (c) 2025, Guo Ruijing (rokkiea)
 *
 * This program has been tested solely on the Orange Pi 5 Pro with the
 * ADS1256. It should theoretically work with the ADS1255 as well.
 * However, its functionality on any other board is not guaranteed.
 *
 ***********************************************************************
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
 * For details of ADS125x, see:
 *  TI ADS125x: https://www.ti.com/product/ADS1256
 *              https://www.ti.com/product/ADS1255
 *  Datasheet: https://www.ti.com/lit/gpn/ads1256
 *             https://www.ti.com/lit/gpn/ads1255
 */

#include <stdint.h>
#include <gpiod.h>

#define ADS125x_DATA_LEN_BYTE 3

typedef struct ads125x_dev_struct
{
    char *name;
    int fd;
    uint8_t spi_mode;
    uint8_t spi_bit_p_word;
    int spi_speed;

    struct gpiod_chip *pin_DRDY_chip;
    struct gpiod_line *pin_DRDY_line;

    struct gpiod_chip *pin_PDWN_chip;
    struct gpiod_line *pin_PDWN_line;
} ads125x_dev;

int FailurePrint(const char *message, ...);
int32_t convert_to_signed_24bit(const unsigned char *result);
int ads125xGetGPIOLine(char *chip, int line, struct gpiod_chip **cp, struct gpiod_line **lp);
int ads125xOpenDRDY(ads125x_dev *dev, char *chip, int line);
int ads125xOpenPDWN(ads125x_dev *dev, char *chip, int line, uint8_t init_status);
void ads125xCloseDRDY(ads125x_dev *dev);
void ads125xwaitDRDY(struct gpiod_line *line);
int SPISetup(const int channel, const int port, const int speed, const int spiBPW, const int mode);
int SPIRelease(const int fd);
int ads125xSetup(ads125x_dev *dev, int spiChannel, int spiPort);
void ads125xSetMUX(ads125x_dev *dev, const uint8_t psel, const uint8_t nsel);
void ads125xSetDRATE(ads125x_dev *dev, const uint8_t dr);
void ads125xSendCMD(ads125x_dev *dev, const uint8_t cmd);
void ads125xRREG(ads125x_dev *dev, const uint8_t regaddr, uint8_t *data, const uint8_t len);
void ads125xWREG(ads125x_dev *dev, const uint8_t regaddr, uint8_t *data, const uint8_t len);
void ads125xRDATA(ads125x_dev *dev, uint8_t *data);
void ads125xRDATAC(ads125x_dev *dev, uint8_t *data, int times);
int ads125xSetPDWN(ads125x_dev *dev, uint8_t status);
// void ads125xSELFCAL(ads125x_dev *dev);
// void ads125xSELFOCAL(ads125x_dev *dev);
// void ads125xSELFGCAL(ads125x_dev *dev);
// void ads125xSYSOCAL(ads125x_dev *dev);
// void ads125xSYSGCAL(ads125x_dev *dev);
// void ads125xRESET(ads125x_dev *dev);
// void ads125xWAKEUP(ads125x_dev *dev);
// void ads125xSTANDBY(ads125x_dev *dev);
