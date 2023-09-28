# 1924B - Mini Black Box
> FLIGHT PATH RECORDING SYSTEM

## Table of Contents
* [General Information](#general-information)
* [Technologies Used](#technologies-used)
* [Features](#features)
* [Configuration](#configuration)
* [Usage](#usage)
* [Project Status](#project-status)
* [Improvements](#improvements)
* [Contact](#contact)
<!-- * [License](#license) -->

## General Information
This project aims to collect and store flight data measurements and location of an aircraft using an inertial unit and a GPS/GNSS positioning system. By combining these technologies, it's possible to record data with remarkable accuracy regarding flight parameters and the aircraft's trajectory. In the event of an incident, these recordings play a crucial role in determining potential causes. For an in-depth analysis of technical specifications, please refer to the complete specification book available in the appendix.
<!-- You don't have to answer all the questions - just the ones relevant to your project. -->

## Technologies Used
- MPLAB Harmony v2_06
- Microcontroller - PIC32MX274F256D
- Inertial Unit - Bosch BNO055
- GNSS - CAM-M8C-0

## Features
• Save inertial data every 500ms by default.
• Save location data every 5,000ms by default.
• Option to configure save times.
• Shock resistant.
• Good battery life / Low power.
• Global Positioning System (GPS).
• Global Navigation Satellite Systems (GNSS).
• Satellite timestamp.
• Inertial Unit.
• Charging, reading, and configuration via USB-C.

## Configuration
The electronic board must be assembled according to the manufacturing files and the BOM (Bill of Materials). The firmware must then be implemented in the PIC microcontroller, and an SD card must be inserted to read the movement data.

## Usage
You need to flash the firmware code through MPLAB-X with harmony on the produced PCB's microcontroller. To do this, there's a BERG programming connector. Once the code is flashed, the black box can be used, and provided an SD card is inserted, movement data will be saved in CSV format.

## Project Status
The project is completed, but improvements can still be made.

## Contact
Created by Ali Zoubir: ali.zoubir@etml-es.ch
