# Firmware updates Over The Air
This project is to enable remote firmware updates using **ESP32**, **STM32F103C8T6** and **Geogle Firebase**.The **ESP32** connects to **WiFi**, downloads the bootloader and firmware code, and communicates with the STM32.
![Screenshot 2024-02-01 104155](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/b0d02c6f-00ba-4343-941e-dbdb82eed83a)


# Table of Contents
1. [Introduction](#Introduction)
2. [Software Interface](#Software-Interface)
3. [Main Features](#Main-Features)
4. [How To Install And Run](#How-To-Install-And_Run)
5. [How To Use](#How-To-Use)
6. [New_Features](#New-Features)

## Introduction
This project facilitates firmware updates over-the-air (FOTA) for an STM32F103 microcontroller using an ESP32 and Google Firebase. Key components include WiFi connectivity, Firebase authentication, and MQTT communication.

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

## How To Use
+ If you want to connect your bootloader with pythonScript that named as **Shehab.py** to communicate with bootloader througth uart without ESP32.<br />
  configure bootloader.h with "python = 1 and ESP = 0" and you can use all main features.<br />

+ If you want to use it as Firmware Over The Air<br />
  configure bootloader.h with "python = 0 and ESP = 1"<br />
  > [NOTE]
  > We dtermine this specific functions to be commiuncate between ESP and STM because the main target of FOTA is uploading applications and erase them and determine thoer protection     
    level.<br />
  > Any other specification you can move into flash in freely movements with **Shehab.py**.<br />

1. Create any project and generate its binary file. Edited its name to **Application.txt**.<br />
2. Use pythonscript that It's named "test.py" in **BinaryFile** folder to track whole file and generates **ApplicationEdited.bin** without any spaces or new lines.<br />
3. Upload **ApplicationEdited.bin** to firebase storage.<br />
![Firebase](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/71ba20b2-ecf9-4a2e-a61c-69737b538e13)
4. **ESP32** will track whole file and save it in it's memory. and wait to action option from web dashboard.<br />
![Dash board](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/8e8f0287-d119-4696-b5f4-07b5df8d2bdf)
5. Press any button that determine your choice.<br />
  - If you pressed **Write Into Memory**: **ESP32** will transmit file to Master MCU.<br />
  - If you pressed **Erase Specific Pages** you must go to **Node Red Layput** as shown:<br />
![NodeRed Layout](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/a5c9a45e-0a03-4c8c-92cb-1ae178b81508)
  - And press double click to MQTT In under name **FOTA/Commends** then enter your page number then press **Deploy**. <br />
  - Press double click to MQTT In under name **FOTA/Commends** then enter your amount of pages then press **Deploy**.<br />
  - Check your this specific page.<br />

## New Features
Master branch will detect if this commend belong to them or not. If It's not belong to it,It will transmit the commend using CAN protocol. The master or branch will response to esp with master with suitable frame to transmit it to firebase with the previous host responses. Finish the future functions and commends that you want. 


