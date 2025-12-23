# Embedded-Game-Systems-Arduino

**Sensor Integration | Pattern Recognition | Inter-Device Communication**

## Project Overview
This repository contains two embedded systems projects focused on creative sensor integration and real-time logic. Both systems functioned as peripheral nodes within a larger "Game Master" network controlled by an ESP32.

---

## Subsystem 1: Capacitive Touch "Whac-A-Mole"
A reaction-speed game utilizing custom built hardware sensors.
* **Hardware:** 3 custom copper plate touch sensors.
* **Logic:** Implemented a timed game loop with  LED signals and a defined pass/fail success threshold.
* **Technical Focus:** Digital signal processing of touch inputs and precise timing management for high speed user interaction.

## Subsystem 2: Acoustic Pattern Recognition (Secret Knock)
A security-logic game based on acoustic input analysis.
* **Hardware:** Piezoelectric vibration sensor.
* **Features:** * **Dynamic Recording:** Users can input and save a new knock sequence as a password.
    * **Logic:** Analyzes the timing and length of the sequence to compare against stored values.
* **Technical Focus:** Analog signal thresholding and array based pattern comparison logic.

## System Integration (ESP32 Communication)
Both projects were designed to function as nodes in a distributed system. 
* **Communication:** Developed the interface logic to transmit game status, success/fail signals, and timing data to a central **ESP32 Game Master**.
* **Skill Demonstrated:** Protocol based communication and system level integration in a multicontroller environment.

* ## Demo Video
https://youtu.be/eFHaTReyPgo?si=uA8e_jXQPgp8DHMi
