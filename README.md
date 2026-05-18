# LVGL Example Simple

![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/InzynierDomu/LVGL_example_simple/main.yml?logo=github&style=flat-square)
![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/InzynierDomu/LVGL_example_simple?style=flat-square)
<a href="https://discord.gg/KmW6mHdg">![Discord](https://img.shields.io/discord/815929748882587688?logo=discord&logoColor=green&style=flat-square)</a>
![GitHub](https://img.shields.io/github/license/InzynierDomu/LVGL_example_simple?style=flat-square)

## Description

Minimal starter project showing how to use LVGL 9 with LovyanGFX on an ESP32 and a 2.4" ILI9341 touchscreen. Displays a title, a click counter and a button — a clean base to build your own touch UI from.

<div align="center">
<h2>Support</h2>

<p>If any of my projects have helped you in your work, studies, or simply made your day better, you can buy me a coffee. <a href="https://buycoffee.to/inzynier-domu" target="_blank"><img src="https://buycoffee.to/img/share-button-primary.png" style="width: 195px; height: 51px" alt="Postaw mi kawę na buycoffee.to"></a></p>
</div>

## Required environment

- **Board**: LOLIN32 (ESP32)
- **Platform**: PlatformIO [video](https://youtu.be/Em9NuebT2Kc)
- **Framework**: Arduino
- **Libraries**: LovyanGFX ^1.1.16, LVGL ^9.1.0

## Hardware

### Parts

- LOLIN32 (or any ESP32 dev board)
- 2.4" ILI9341 TFT display with XPT2046 touch controller (320×240, SPI)

### Wiring

| Signal      | ESP32 GPIO |
|-------------|-----------|
| SPI SCLK    | 18        |
| SPI MOSI    | 23        |
| SPI MISO    | 19        |
| Display DC  | 16        |
| Display CS  | 5         |
| Display RST | 17        |
| Touch CS    | 15        |
| Touch INT   | 34        |
