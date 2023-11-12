# Firmware updates Over The Air
This project is to enable remote firmware updates using **ESP8266**, **STM32F103C8T6** and **Geogle Firebase**.The **ESP8266** connects to **WiFi**, downloads the bootloader and firmware code, and communicates with the STM32.

# Table of Contents
1. [Introduction](#Introduction)
2. [Software Interface](#Software_Interface)
3. [Main Features](#Main_Features)
4. [How To Install And Run](#How_To_Install_And_Run)
5. [How To Use](#How_To_Use)
   
## 1. Introduction
This project facilitates firmware updates over-the-air (FOTA) for an STM32F103 microcontroller using an ESP8266 and Google Firebase. Key components include WiFi connectivity, Firebase authentication, and MQTT communication.

## 2. Software Interface
This section provides an overview of the primary functions and interfaces used in this project.

## 3. Main Features
### Current Features
+ **Get software version**: Vendor Id, Major version, Minor version, Patch Id.
+ **Get help function**: Get help commends that we will use in dealing with bootloader.
+ **Get Chip ID**: Get the identification id of our chip.
+ **Read protection level**: Read the protection level of flash memory.
+ **Jump to address**: Jump to specific address if it's application address or any one other to implement this function 
+ **Erase flash**: To erase specific sectors in flash memory
+ **Memory write**: To write on specific pages in flash memory.
### Worked On
+ **Enable/Disable write protection**: protect specific pages from writing on it or disabled.
+ **Read Memory**: Read specific bytes in memory.
+ **Read Page Status**: Read the status of this page (protection level).
+ **Change protection level**: Change the protection level of flash memory.
  
## 4. How To Install And Run

## 5. How To Use



