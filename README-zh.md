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

## 从源码编译

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

## 性能测试

| 目标速率/SPS | 采样数量 | 消耗时间/s | 实际速率/SPS | 与目标速率之比/% |
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
| 50       | 250    | 4.9993 | 50.8071  | 101.6142  |
| 30       | 150    | 4.9993 | 30.0042  | 100.0140  |
| 25       | 125    | 4.9993 | 25.0035  | 100.0140  |
| 15       | 75     | 4.9993 | 15.0021  | 100.0140  |
| 10       | 50     | 4.9993 | 10.0014  | 100.0140  |
| 5        | 25     | 4.9993 | 5.0007   | 100.0140  |
| 2.5      | 25     | 9.9989 | 2.5003   | 100.0120  |

该驱动程序在大于 15kSPS 时会遇到性能不佳的问题。以驱动目前的实现方式这似乎是无解的，这是由于 Linux 的各种 sleep 计时函数并不那么准确，因此驱动采用了轮询执行 `gpiod_line_get_value` 的方式来获取 DRDY 的状态。但这也导致在高频率的 RDATAC 期间 CPU 会接近 100% 占用。而且用户态的驱动程序通常会与其他程序出现资源竞争，还会受到操作系统调度器的影响，在针对采样速率和精度要求均极高的场景，本驱动并不合适。

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