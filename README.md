# Microcontroller-based Morse Code Encoder and Decoder

## Team Members

- Noble Carpenter ([@NobleCarpenter](https://github.com/NobleCarpenter))
- Teagan Tobias ([@ttobias28](https://github.com/ttobias28))
  
---
## Project Overview

This project implements a dual-mode Morse code interface developed for the STM32 NUCLEO-L476RG and the EDUBase V2 board. The system facilitates seamless Morse code communication through two distinct workflows: a keypad-to-signal Encoder and a tactile Decoder that utilizes hardware timers to translate manual button presses into alphanumeric characters.

---
## Project Capabilities and Goals
- Dual-Mode Operations: functions as both a Morse code transmitter and receiver.
- Hybrid Display: outputs decoded text across both an LCD screen and a 7-segment display.
- Precise Pulse Detection: uses hardware timers to accurately measure and distinguish dit/dah durations.
- Translation Accuracy: ensure 1:1 mapping between International Morse Code and alphanumeric characters.
- Hardware Efficiency: optimize peripheral interfacing (GPIO, Timers, LCD) on the STM32 platform.

---
## Software and Hardware Included
Software:

- Keil uVision5, programming in C code.

Hardware: 

- STM32 NUCLEO-L476RG
- EDUBase V2
