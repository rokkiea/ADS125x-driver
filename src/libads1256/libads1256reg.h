/**
 * libads1256reg.h - TI ADS1256 driver
 * 
 *	Extend wiringPi with the ADS1256 SPI 24-Bit ADC
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
 * For details on all registers and their usage, please refer to the 
 * datasheet provided by Texas Instruments.
 *  TI ADS1256: https://www.ti.com/product/ADS1256
 *  Datasheet: https://www.ti.com/lit/gpn/ads1256
 */

// DEFAULT CONFIG

// Register address
#define ADS125x_REG_ADDR_STATUS             0x00
#define ADS125x_REG_ADDR_MUX                0x01
#define ADS125x_REG_ADDR_ADCON              0x02
#define ADS125x_REG_ADDR_DRATE              0x03
#define ADS125x_REG_ADDR_IO                 0x04
#define ADS125x_REG_ADDR_OFC0               0x05
#define ADS125x_REG_ADDR_OFC1               0x06
#define ADS125x_REG_ADDR_OFC2               0x07
#define ADS125x_REG_ADDR_FSC0               0x08
#define ADS125x_REG_ADDR_FSC1               0x09
#define ADS125x_REG_ADDR_FSC2               0x0A

// STATUS register
#define ADS125x_STATUS_ORDER_MSB            0x00    // default
#define ADS125x_STATUS_ORDER_LSB            0x04
#define ADS125x_STATUS_ACAL_ON              0x00
#define ADS125x_STATUS_ACAL_OFF             0x03    // default
#define ADS125x_STATUS_BUFEN_ON             0x00    // default
#define ADS125x_STATUS_BUFEN_OFF            0x02
#define ADS125x_STATUS_DRDY                 0x01    // DRDY is read-only

// MUX register
/**
 * Input & Output Channel Select
 *  PSEL represents MUX[7:4], and NSEL represents MUX[3:0]. The values
 *  of PSEL and NSEL must be ORed together before being written to the
 *  MUX register.
 */
#define ADS125x_MUX_PSEL_CH0                0x00   // default
#define ADS125x_MUX_PSEL_CH1                0x10
// Below is ADS1256 only
#define ADS125x_MUX_PSEL_CH2                0x20
#define ADS125x_MUX_PSEL_CH3                0x30
#define ADS125x_MUX_PSEL_CH4                0x40
#define ADS125x_MUX_PSEL_CH5                0x50
#define ADS125x_MUX_PSEL_CH6                0x60
#define ADS125x_MUX_PSEL_CH7                0x70
// Above is ADS1256 only

#define ADS125x_MUX_NSEL_CH0                0x00
#define ADS125x_MUX_NSEL_CH1                0x01    // default
// Below is ADS1256 only
#define ADS125x_MUX_NSEL_CH2                0x02
#define ADS125x_MUX_NSEL_CH3                0x03
#define ADS125x_MUX_NSEL_CH4                0x04
#define ADS125x_MUX_NSEL_CH5                0x05
#define ADS125x_MUX_NSEL_CH6                0x06
#define ADS125x_MUX_NSEL_CH7                0x07
// Above is ADS1256 only

// ADCON register
// ADCON_CLK
#define ADS125x_ADCON_CLK_OFF               0x00
#define ADS125x_ADCON_CLK_FEQIN             0x20    // default
#define ADS125x_ADCON_CLK_FEQIN_2           0x40
#define ADS125x_ADCON_CLK_FEQIN_4           0x60

// ADCON_SDCS
#define ADS125x_ADCON_SDCS_OFF              0x00    // default
#define ADS125x_ADCON_SDCS_0_5              0x08
#define ADS125x_ADCON_SDCS_2                0x10
#define ADS125x_ADCON_SDCS_10               0x18

// ADCON_PGA
#define ADS125x_ADCON_PGA_1                 0x00    // default
#define ADS125x_ADCON_PGA_2                 0x01
#define ADS125x_ADCON_PGA_4                 0x02
#define ADS125x_ADCON_PGA_8                 0x03
#define ADS125x_ADCON_PGA_16                0x04
#define ADS125x_ADCON_PGA_32                0x05
#define ADS125x_ADCON_PGA_64                0x06

// DRATE register
#define ADS125x_DR_2_5                      0x03
#define ADS125x_DR_5                        0x13
#define ADS125x_DR_10                       0x23
#define ADS125x_DR_15                       0x33
#define ADS125x_DR_25                       0x43
#define ADS125x_DR_30                       0x53
#define ADS125x_DR_50                       0x63
#define ADS125x_DR_60                       0x72
#define ADS125x_DR_100                      0x82
#define ADS125x_DR_500                      0x92
#define ADS125x_DR_1000                     0xA1
#define ADS125x_DR_2000                     0xB0
#define ADS125x_DR_3750                     0xC0
#define ADS125x_DR_7500                     0xD0
#define ADS125x_DR_15000                    0xE0
#define ADS125x_DR_30000                    0xF0    // default

// GPIO Control register
#define ADS125x_IO_DIR3_IN                  0x80    // default
#define ADS125x_IO_DIR2_IN                  0x40    // default
#define ADS125x_IO_DIR1_IN                  0x20    // default
#define ADS125x_IO_DIR0_IN                  0x10    // default is OUT, 0

// Command Definitions
#define ADS125x_CMD_WAKEUP                  0x00
#define ADS125x_CMD_RDATA                   0x01
#define ADS125x_CMD_RDATAC                  0x03
#define ADS125x_CMD_SDATAC                  0x0F
// low bits is reg addr
#define ADS125x_CMD_RREG                    0x10
// low bits is reg addr
#define ADS125x_CMD_WREG                    0x50    
#define ADS125x_CMD_SELFCAL                 0xF0
#define ADS125x_CMD_SELFOCAL                0xF1
#define ADS125x_CMD_SELFGCAL                0xF2
#define ADS125x_CMD_SYSOCAL                 0xF3
#define ADS125x_CMD_SYSGCAL                 0xF4
#define ADS125x_CMD_SYNC                    0xFC
#define ADS125x_CMD_STANDBY                 0xFD
#define ADS125x_CMD_RESET                   0xFE
