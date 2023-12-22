# Firmware updates Over The Air
This project is to enable remote firmware updates using **ESP8266**, **STM32F103C8T6** and **Geogle Firebase**.The **ESP8266** connects to **WiFi**, downloads the bootloader and firmware code, and communicates with the STM32.

# Table of Contents

1. [Introduction](#Introduction)
2. [Software Interface](#Software-Interface)
3. [Main Features](#Main-Features)
4. [How To Install And Run](#How-To-Install-And_Run)
5. [How To Use](#How-To-Use)
6. [New_Features](#New-Features)

## Introduction
This project facilitates firmware updates over-the-air (FOTA) for an STM32F103 microcontroller using an ESP8266 and Google Firebase. Key components include WiFi connectivity, Firebase authentication, and MQTT communication.

## Software Interface
This section provides an overview of the primary functions and interfaces used in this project.

## Main Features
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
  
## How To Install And Run

## How To Use

## New Features
Task 1:</br>
Make BootManager in page(0:n) in flash memory to stay in managing. Web server send commend to BootManager to connect on filezille server using esp32 that recieve file.bin "BootLoader" to write it in flash page(n:m) then set the managing flag and reset micro-controller. Bootmanager ask about managing flag at first if It set jump to Bootloader. If not It will stay in managing. After it will be setted protect this region with level 1.</br>
Task2:</br>
Bootloader stays in booting till recieve any commend from web server. Web server can send any commend to esp options and make caculations on it as CRC and make suitable frames to bootloader. Bootlaoder reply on server using esp32 (on firbase or server) with the wanted response. When server send commend to connect on filezille then takes file.bin "Application" to write it in flash page(n:m) starting from entered address then set the booting flag and reset micro-controller. BootLoader ask about booting flag at first. If not it will stay in booting. If It set Bootloader will check on region of flags that contains application flags. </br>
Task3:</br>
Application when notices accumalting errors in code that will clear application flag then reset micro-controller. If it is resetted, BootManager will check on managing flag to be sure that is ok. Then will jump to bootloader and check before while(1) on booting flag. If it is setted, It will chack on application flags to read which one is ok and jump on it's application.</br>
Task4:</br>
Web server should send if this commends is related to master micro-controller or branch. If it relates to master, We will do the previos senario. If it doesn't relate, It will communicates with the branch with CAN protocal with the same senario. </br>
