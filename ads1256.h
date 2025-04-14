/**
 * ads1256.h - TI ADS125x driver
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

#define ADS125x_SPI_SPEED 1920000
#define ADS125x_SPI_MODE SPI_MODE_1
#define ADS125x_SPI_BIT_P_WORD 8
#define ADS125x_DRDY_CHIP "gpiochip1"
#define ADS125x_DRDY_LINE 3
#define ADS125x_PDWN_CHIP "gpiochip4"
#define ADS125x_PDWN_LINE 3
