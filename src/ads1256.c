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

extern int ADS125xDriverDebug;
char *usage = "Usage: [options...]\n"
              " -h, --help                 Show this manual\n"
              " -s, --single               Single read\n"
              " -c, --continuous <times>   Read data 'times' times in continuous mode.\n"
              "     -o, --output <file>    Write continuous mode data to a file\n"
              " -p, --pdwn [off/on/0/1]    Set PDWN low (off/0) or high (on/1)\n\n"
              "ads1256 homepage at: https://github.com/rokkiea/ADS125x-driver\n"
              "Copyright (c) 2025 Guo Ruijing (rokkiea)";

void one_shot_read();
void continu_read(FILE *output, int times);
void doContinuRead(int argc, char* argv []);
void doPdwn(int argc, char* argv []);

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
    ads125xRESET(&ads1256);
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
    return;
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
    return;
}

void doContinuRead(int argc, char* argv [])
{
    FILE *fp = NULL;
    if (argc < 3) {
        fprintf (stderr, "Usage: %s -c/--continuous <times>\n", argv [0]) ;
        exit (1);
    }
    else if (argc == 4 || argc > 5)
    {
        fprintf (stderr, "Usage: %s -c/--continuous <times> -o <files>\n", argv[0]);
        exit (1);
    }

    switch (argc)
    {
        case 5: 
            if((fp = fopen(argv[4], "w")) == NULL)
            {
                fprintf(stderr, "Open file %s error.\n", argv[4]);
                exit(EXIT_FAILURE);
            }
            continu_read(fp, atoi(argv[2])); break;
        case 3:
        default: continu_read(stdout, atoi(argv[2])); break;
    }
    return;
}

void doPdwn(int argc, char* argv [])
{
    int ret = 0;
    ads125x_dev ads1256;

    if (argc == 2 || argc > 3) {
        fprintf (stderr, "Usage: %s -p/--pdwn [off/on/0/1]\n", argv [0]) ;
        exit (1) ;
    }

    if ((ret = ads125xOpenPDWN(&ads1256, ADS125x_PDWN_CHIP, ADS125x_PDWN_LINE, 0)))
        fprintf(stderr, "Open PDWN err: %d\n", ret);
    // Set PDWN to high to POWER-UP ADS1256
    
    /**/ if ( strcasecmp(argv[2], "off") == 0 || strcasecmp(argv[2], "0") == 0 ) ads125xSetPDWN(&ads1256, 0);
    else if ( strcasecmp(argv[2], "on") == 0 || strcasecmp(argv[2], "1") == 0 ) ads125xSetPDWN(&ads1256, 1);
    else {
        fprintf(stderr, "Invalid PDWN status %s .\n", argv[2]);
        ads125xClosePDWN(&ads1256);
        exit(EXIT_FAILURE);
    }
    ads125xClosePDWN(&ads1256);
    return;
}

int main(int argc, char *argv[])
{
    char *env = NULL;
    if ((env = getenv("ADS1256_DRIVER")) != NULL)
        if (0 == atoi(env))
            ADS125xDriverDebug = true;

    if (argc == 1)
    {
        fprintf(stderr,
                "%s: Please provide the arguments.\n"
                "  Type: ads1256 -h for full details.\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcasecmp(argv[1], "-h") == 0 || strcasecmp(argv[1], "--help") == 0)
    {
        fprintf(stdout, "%s: %s\n", argv[0], usage);
        exit(EXIT_SUCCESS);
    }

    if (geteuid() != 0)
    {
        fprintf(stderr, "%s: Must be root to run. Program should be suid root. This is an error.\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /**/ if ( strcasecmp (argv[1], "-s") == 0 || strcasecmp (argv[1], "--single"    ) == 0 ) one_shot_read();
    else if ( strcasecmp (argv[1], "-c") == 0 || strcasecmp (argv[1], "--continuous") == 0 ) doContinuRead(argc, argv);
    else if ( strcasecmp (argv[1], "-p") == 0 || strcasecmp (argv[1], "--pdwn") == 0)        doPdwn(argc, argv);
    else if ( strcasecmp (argv[1], "-o") == 0 || strcasecmp (argv[1], "--pdwn") == 0)
        {fprintf(stderr, "output parameter can only be used with continuous output.\n"); exit(1);}
    else {
        fprintf (stderr, "%s: Unknown command: %s.\n", argv [0], argv [1]) ;
        exit (EXIT_FAILURE) ;
    }
    return 0;
}