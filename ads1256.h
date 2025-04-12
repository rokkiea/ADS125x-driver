/**
 * ads1256.h - TI ADS1256 driver
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
 * For details of ADS1256, see:
 *  TI ADS1256: https://www.ti.com/product/ADS1256
 *  Datasheet: https://www.ti.com/lit/gpn/ads1256
 */

 #define ADS125x_SPI_SPEED 1000000
 #define ADS125x_SPI_MODE SPI_MODE_1
 #define ADS125x_SPI_BIT_P_WORD 8
 
 typedef struct
 {
     char *name;
     int fd;
     uint8_t spi_mode;
     uint8_t spi_bit_p_word;
     uint8_t spi_speed;
 } ads125x_dev;
 
 int SPIOpen(int channel, int port, int speed, int mode);