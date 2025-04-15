# TI ADS125X Linux driver - 德州仪器 ADS1256 Linux 驱动

[English](./README.md) [简体中文](./README-zh.md)

> *该驱动要求在您的系统中安装 `libgpiod` 库.*

该驱动程序在用户态运行，由于程序并使用中断处理程序，而是使用 循环来检查 DRDY 状态，因此只有在系统接近空载的情况下才有最大性能。该驱动的连续读取性能最高只能达到25kSPS，因此设置为15kSPS及以下的采样速率其表现是最稳定的。

这个程序仅在运行 Debian 12 的 Orange Pi 5 Pro 上 搭配 ADS1256 测试过，理论上使用 ADS1255 也可以正常工作。但不保证在任何其他板子上能够使用。

关于 ADS1256 ，参见: [TI ADS1256](https://www.ti.com/product/ADS1256), [数据表](https://www.ti.com/lit/gpn/ads1256)

---

## 简介

`libads1256` 为驱动函数库，里面提供了一组控制 TI ADS1256 的函数。

`ads1256.c` 为示例程序，您可参照程序中的编写思路编写您自己的驱动程序。

## Compile from source

* 假设您的环境为运行 Debian 12 发行版的香橙派 5 Pro。

1. 安装 `git`, `make`, `gcc`, 和 `libgpiod-dev`

    `sudo apt install git make gcc libgpiod-dev`

2. 克隆本仓库

    `git clone https://github.com/rokkiea/ADS125x-driver.git`

3. 切换至仓库目录并编译

    ```shell
    cd ADS125x-driver
    make
    ```

4. 编译输出为 `ads1256`

## 使用示例

- 进行一次单次采样

    `./ads1256 -s`

- 进行 100 次连续采样

    `./ads1256 -c 100`

- 进行连续采样并输出到文件

    `./ads1256 -c 100 -o output.csv`

- 也可以设置 `PDWN` 引脚电平

    `./ads1256 -p off`

- 您也可以在程序中直接查看用法

    ```shell
    $ ./ads1256 -h
    ./ads1256: Usage: [options...] -h, --help                 Show this manual
    -s, --single               Single read
    -c, --continuous <times>   Read data 'times' times in continuous mode.
        -o, --output <file>    Write continuous mode data to a file
    -p, --pdwn [off/on/0/1]    Set PDWN low (off/0) or high (on/1)

    ads1256 homepage at: https://github.com/rokkiea/ADS125x-driver
    Copyright (c) 2025 Guo Ruijing (rokkiea)
    ```

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