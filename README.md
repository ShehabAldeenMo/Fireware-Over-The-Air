# Firmware updates Over The Air
This project is to enable remote firmware updates using **ESP32**, **STM32F103C8T6** and **Geogle Firebase**.The **ESP32** connects to **WiFi**, downloads the bootloader and firmware code, and communicates with the STM32.
![Screenshot 2024-02-01 104155](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/b0d02c6f-00ba-4343-941e-dbdb82eed83a)
<br />
<br />

# Table of Contents
1. [Introduction](#Introduction)
2. [Hardware Connection](#Hardware-Connection)
3. [Problems And Solutions](#Problems-And-Solutions)
4. [Main Features](#Main-Features)
7. [New_Features](#New-Features)
<br />

## Introduction
The technological evolution of a car has been in sync with rapid advances in communication, information processing and electronic hardware systems. Starting with isolated Electronic Control Units (ECUs) for Engine Management and Anti Lock Braking, typical cars now use from 25 to 100 microcontrollers for providing many features to ensure comfort and safety to the driver and passengers. The ECUs used in the car, for handling its overall function, are controlled by complex software code called as the firmware. With a large number of ECUs present in a car, the size of code in a modern luxury car can exceed 100 million lines of code as compared to 5 to 6 million lines of code required for a supersonic fighter aircraft. It is obvious that maintaining and updating such large volume of code for several thousand vehicles in production and millions in the field is a mammoth task. Recalling vehicles in the field to correct field problems, to introduce new features and to upgrade vehicle performance can cause loss of reputation for the OEM and incurrence of huge costs. Hence a methodology to update firmware of automotive ECUs automatically in an error-free manner and at a faster rate is necessary to support programming and updating of firmware of a modern car during its life cycle.<br /><br />

FOTA (Firmware Over The Air) is emerging as a new flexible solution for these problems. FOTA is already an established technology for mobile phones and its application to automotive domain helps to tackle rising complexity of automotive systems. FOTA is based on the wireless communication between firmware server located in Cloud and the Telematics Control Unit used in a car as a client to download the new firmware. FOTA can update firmware of a car at any location whether it is an assembly shop, a dealer location, a service workshop or the parking space of the owner. Besides, FOTA allows download of firmware in the background when the vehicle is running and informs the driver, owner or service person when the installation of the updated firmware can be started. <br /><br />

A mention must be made of another important remote programming process viz. SOTA (Software Over The Air) for automotive infotainment systems. The process is well established and uses same wireless network for downloading updates. However, the important difference between FOTA and SOTA is in its contents and the target for its installation. In contrast to FOTA for critical control systems, SOTA is used to update maps in Telematics unit, games & music in infotainment systems etc; SOTA is thus confined to non-critical aspects of vehicle operation.<br /><br />

## Hardware Connection
+ Connections Between ESP and Master ECU <br />
  TX2 RX2 in esp <--> RX1 TX1 in master ECU. <br />
+ Connections Between Master ECU and Slave1 <br />
  TX2 RX2 in master ECU  <--> RX1 TX1 in Slave1. <br />
+ Battery system <br />
  1- Common GND between ECU's <br />
  2- Common +5V between ECU's <br />
  3- <br />
   ![1](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/7fd09093-9b70-4d22-869e-dbd47c58c8d5)
<br />

## Problems And Solutions
+ How we can send our commends ?<br />
Using NodeRed to set specific buttons and its payload.<br />
![2](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/0067602c-42fd-44ba-9284-88b8213e6ab9)
To creat this DashBoard.<br />
![4](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/f0a3ac6f-07d1-4455-a9f5-8809f6f2854b)
![3](https://github.com/ShehabAldeenMo/Fireware-Over-The-Air/assets/114957788/46e2c23c-9ef1-4cb2-b91d-d4a114f3f039)
<br />

+ How this Dashboard could be worth to our system ? <br />
Create ESP project on arduino ide on MQTT example on mosquitto server and make our ESP subscribe speific topic like "FOTA/FromBrokerToEsp". <br />
Make our ESP publish on another topic like "FOTA/FromEspToBroker" and make debug button on node-red to synchronize the debug info to mqtt.  <br />
<br />

+ How ESP could affect on master ECU ?  <br />
ESP get each string from broker and based on it , ESP will form specific frames suitable for bootloader and send it thruogh uart.
 <br />
 
+ How Master ECU do communicate with other branchs ? <br />
using uart connections.
 <br />
 
+ How to handle application on ECU ?  <br />
At first we send our program by commend from mqtt to ESP called "Write program". <br />
ESP will download file from **Firebase** and send it to stm in speific address that you specify it before through dashboard. <br />
Through dashboard, We will send order to jump to application. Bootloader will write on application1 flag in the last page that the application is valid then jump to application. <br />
Each reset in system bootloader asks at first about this flag. If it is valid, Bootloader will jump on it. <br />
If application receive order to jump to bootloader, We will clear this flag and reset our reset. <br />
 <br />
 
+ How to prevent hacker to write or read on our ECU that already written on Flash memory ? <br />
We will make protection level of our flash memory level 1 in write and read.  <br />  <br />

## Main Features
### Current Features
+ **Get software version**: Vendor Id, Major version, Minor version, Patch Id.
+ **Read protection level**: Read the protection level of flash memory.
+ **Jump to address**: Jump to specific address if it's application address or any one other to implement this function 
+ **Erase flash**: To erase specific sectors in flash memory
+ **Memory write**: To write on specific pages in flash memory.
+ **Enable/Disable write protection level**: protect flash memory from writing on it or disabled.
+ **Enable/Disable read protection level**: protect  flash memory from reading on it or disabled.

## New Features
Master branch will detect if this commend belong to them or not. If It's not belong to it,It will transmit the commend using CAN protocol. Use openssl in encrption and decryption files. 


