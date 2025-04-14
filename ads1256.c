/**
 * ads1256.c - TI ADS1256 driver
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
// #define _GNU_SOURCE
#include <byteswap.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

#include "libads1256.h"
#include "libads1256reg.h"
#include "ads1256.h"

#define DEV_SPI_SPEED 1920000 // 1MHz
#define DEV_SPI_BIT_P_WORD 8

void one_shot_read();
void continu_read(FILE *output, int times);

void one_shot_read()
{
    uint8_t result[4] = {0};
    int i = 0;
    int ret = 0;
    uint64_t calcResult = 0;
    double result_volt = 0;
    ads125x_dev ads1256;

    // Init ads1256 struct memory space
    memset(&ads1256, 0x00, sizeof(ads1256));
    ads1256.name = "ADS1256";
    ads1256.spi_mode = ADS125x_SPI_MODE;
    ads1256.spi_bit_p_word = ADS125x_SPI_BIT_P_WORD;
    ads1256.spi_speed = ADS125x_SPI_SPEED;

    // Setup SPI bus
    if (0 == (ads1256.fd = ads125xSetup(&ads1256, 0, 0)))
    {
        printf("SPI setup failed.\n");
        exit(EXIT_FAILURE);
    }

    // Open DRDY & PDWN
    if ((ret = ads125xOpenDRDY(&ads1256, ADS125x_DRDY_CHIP, ADS125x_DRDY_LINE)))
        fprintf(stderr, "Open DRDY err: %d\n", ret);
    if ((ret = ads125xOpenPDWN(&ads1256, ADS125x_PDWN_CHIP, ADS125x_PDWN_LINE, 0)))
        fprintf(stderr, "Open PDWN err: %d\n", ret);

    // Set PDWN to high to POWER-UP ADS1256
    ads125xSetPDWN(&ads1256, 1);

    // RESET ADS1256 to clean previous settings and status
    ads125xSendCMD(&ads1256, ADS125x_CMD_RESET);
    // Set data rate to 15000 sps
    ads125xSetDRATE(&ads1256, (uint8_t)ADS125x_DR_15000);
    // Set Multiplexer, the result will be V_CH0 - V_CH1
    ads125xSetMUX(&ads1256, ADS125x_MUX_PSEL_CH0, ADS125x_MUX_NSEL_CH1);
    // Requite Self Offset and Gain Calibration
    ads125xSendCMD(&ads1256, ADS125x_CMD_SELFCAL);

    // Read Register STATUS, MUX, ADCON, DRATE
    ads125xRREG(&ads1256, ADS125x_REG_ADDR_STATUS, result, 0x04);
    fprintf(stdout, "STATUS MUX ADCON DRATE REG: ");
    for (i = 0; i < 4; ++i)
        fprintf(stdout, "%02hx ", result[i]);
    fprintf(stdout, "\n");

    // one-shot read data
    ads125xRDATA(&ads1256, result);
    fprintf(stdout, "One-shot:   raw 0x");
    for (i = 0; i < 3; ++i)
        fprintf(stdout, "%02hx", result[i]);

    // convert raw data to real voltage
    calcResult = (uint64_t)convert_to_signed_24bit(result);
    calcResult *= 5;
    result_volt = (double)calcResult / (1ULL << 23);
    fprintf(stdout, "   Volt=%.12lf\n", result_volt);

    // Release all resource
    ads125xSetPDWN(&ads1256, 0);
    ads125xCloseDRDY(&ads1256);
    SPIRelease(ads1256.fd);
}

void continu_read(FILE *output, int times)
{
    uint8_t result[4] = {0};
    uint8_t *rdatac_result = NULL;
    int i = 0, j = 0;
    int ret = 0;
    uint64_t calcResult = 0;
    double result_volt = 0;
    ads125x_dev ads1256;

    // Init ads1256 struct memory space
    memset(&ads1256, 0x00, sizeof(ads1256));
    if ((rdatac_result = (uint8_t *)malloc(times * ADS125x_DATA_LEN_BYTE)) == NULL)
    {
        fprintf(stderr, "Allocated memory for rdatac_result failed.\n");
        exit(1);
    }
    memset(rdatac_result, 0x00, sizeof(times * ADS125x_DATA_LEN_BYTE));
    ads1256.name = "ADS1256";
    ads1256.spi_mode = ADS125x_SPI_MODE;
    ads1256.spi_bit_p_word = ADS125x_SPI_BIT_P_WORD;
    ads1256.spi_speed = ADS125x_SPI_SPEED;

    // Setup SPI bus
    if (0 == (ads1256.fd = ads125xSetup(&ads1256, 0, 0)))
    {
        printf("SPI setup failed.\n");
        exit(EXIT_FAILURE);
    }

    // Open DRDY & PDWN
    if ((ret = ads125xOpenDRDY(&ads1256, ADS125x_DRDY_CHIP, ADS125x_DRDY_LINE)))
        fprintf(stderr, "Open DRDY err: %d\n", ret);
    if ((ret = ads125xOpenPDWN(&ads1256, ADS125x_PDWN_CHIP, ADS125x_PDWN_LINE, 0)))
        fprintf(stderr, "Open PDWN err: %d\n", ret);

    // Set PDWN to high to POWER-UP ADS1256
    ads125xSetPDWN(&ads1256, 1);

    // RESET ADS1256 to clean previous settings and status
    ads125xSendCMD(&ads1256, ADS125x_CMD_RESET);
    // Set data rate to 1000 sps
    ads125xSetDRATE(&ads1256, (uint8_t)ADS125x_DR_1000);
    // Set Multiplexer, the result will be V_CH0 - V_CH1
    ads125xSetMUX(&ads1256, ADS125x_MUX_PSEL_CH0, ADS125x_MUX_NSEL_CH1);
    // Requite Self Offset and Gain Calibration
    ads125xSendCMD(&ads1256, ADS125x_CMD_SELFCAL);

    // Read Register STATUS, MUX, ADCON, DRATE
    ads125xRREG(&ads1256, ADS125x_REG_ADDR_STATUS, result, 0x04);
    fprintf(stdout, "STATUS MUX ADCON DRATE REG: ");
    for (i = 0; i < 4; ++i)
        fprintf(stdout, "%02hx ", result[i]);
    fprintf(stdout, "\n");

    // continues read data
    ads125xRDATAC(&ads1256, rdatac_result, times);
    fprintf(stdout, "====== Continues read ======\n");
    for (j = 0; j < times; ++j)
    {
        fprintf(output, "%5d,", j + 1);
        for (i = 0; i < 3; ++i)
        {
            fprintf(output, "%02hx", rdatac_result[j * 3 + i]);
        }
        calcResult = (uint64_t)convert_to_signed_24bit(rdatac_result + (j * 3));
        calcResult *= 5;
        result_volt = (double)calcResult / (1ULL << 23);
        fprintf(output, ",%.12lf\n", result_volt);
    }

    // Release all resource
    ads125xSetPDWN(&ads1256, 0);
    ads125xCloseDRDY(&ads1256);
    SPIRelease(ads1256.fd);
}

int main(void)
{
    one_shot_read();
    continu_read(stdout, 1000);
    return 0;
}