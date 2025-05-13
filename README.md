# TI ADS125X Linux driver

[English](./README.md) [简体中文](./README-zh.md)

> *This driver requires the `libgpiod` library to be installed on your system.*

This driver operates in user space. Due to the program's reliance on polling the DRDY status in a loop instead of using an interrupt handler, maximum performance is achievable only under near-idle system conditions. The continuous read performance of this driver is limited to a maximum of 25kSPS, therefore setting the sampling rate to 15kSPS or below yields the most stable performance.


This program has only been tested with the ADS1256 on an Orange Pi 5 Pro running Debian 12. It should theoretically work with the ADS1255 as well. However, its functionality on any other board is not guaranteed.

For details of ADS1256, see: [TI ADS1256](https://www.ti.com/product/ADS1256), [Datasheet](https://www.ti.com/lit/gpn/ads1256)

---

## Introduction

`libads1256` is a driver library that provides a set of functions to control the TI ADS1256.

`ads1256.c` is a sample program. You can refer to the programming ideas in the program to write your own driver.

## Compile from source

* Assuming your environment is an Orange Pi 5 Pro running the Debian 12 distribution.

1. Install `git`, `make`, `gcc`, and `libgpiod-dev`
    
    `sudo apt install git make gcc libgpiod-dev`

2. Clone this repository

    `git clone https://github.com/rokkiea/ADS125x-driver.git`

3. Change to the repository directory and compile.

    ```shell
    cd ADS125x-driver
    make
    ```

4. The compiled output is `ads1256`.

## Example Usage

- Perform a single conversion.

    `./ads1256 -s`

- Perform 100 continuous conversions.

    `./ads1256 -c 100`

- Continuous conversions can be output to a file.

    `./ads1256 -c 100 -o output.csv`

- The PDWN pin level can be set.

    `./ads1256 -p off`

- You can also view the usage directly in the program.

    ```TEXT
    $ ./ads1256 -h
    ./ads1256: Usage: [options...]
     -h, --help                 Show this manual
     -s, --single               Single read
     -c, --continuous <times>   Read data 'times' times in continuous mode.
         -o, --output <file>    Write continuous mode data to a file
     -p, --pdwn [off/on/0/1]    Set PDWN low (off/0) or high (on/1)

    ads1256 homepage at: https://github.com/rokkiea/ADS125x-driver
    Copyright (c) 2025 Guo Ruijing (rokkiea)
    ```

## Performance Testing

| Target Rate / SPS | Number of Samples | Elapsed Time / s | Actual Rate / SPS | Ratio to Target Rate / % |
| -------- | ------ | ------ | -------- | --------- |
| 30k      | 150000 | 5.4370 | 27588.74 | 91.9625   |
| 15k      | 75000  | 5.0286 | 14914.69 | 99.4313   |
| 7500     | 37500  | 4.9995 | 7500.750 | 100.0100  |
| 3750     | 18750  | 4.9995 | 3750.375 | 100.0100  |
| 2000     | 10000  | 4.9988 | 2000.480 | 100.0240  |
| 1000     | 5000   | 4.9996 | 1000.080 | 100.0080  |
| 500      | 2500   | 4.9993 | 500.0700 | 100.0140  |
| 100      | 500    | 4.9993 | 100.0140 | 100.0140  |
| 60       | 300    | 4.9993 | 60.0084  | 100.0140  |
| 50       | 250    | 4.9993 | 50.0070  | 100.0140  |
| 30       | 150    | 4.9993 | 30.0042  | 100.0140  |
| 25       | 125    | 4.9993 | 25.0035  | 100.0140  |
| 15       | 75     | 4.9993 | 15.0021  | 100.0140  |
| 10       | 50     | 4.9993 | 10.0014  | 100.0140  |
| 5        | 25     | 4.9993 | 5.0007   | 100.0140  |
| 2.5      | 25     | 9.9989 | 2.5003   | 100.0120  |

The driver encounters performance issues at rates greater than 15 kSPS. With the current implementation of the driver, this appears to be unresolvable. This is due to the inaccuracy of various sleep timing functions in Linux, which forces the driver to poll the DRDY status using `gpiod_line_get_value`. However, this also leads to near 100% CPU utilization during high-frequency RDATAC mode. Furthermore, user-space drivers are generally subject to resource contention with other programs and are affected by the operating system scheduler. Therefore, this driver is not suitable for scenarios with extremely high demands for both sampling rate and accuracy.

## License

Copyright (c) 2025, Guo Ruijing (rokkiea)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.