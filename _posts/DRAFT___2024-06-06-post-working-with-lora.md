---
title: "Working with LoRa"
categories:
  - Hardware
tags:
  - LoRa, Rpi, Arduino
---


Long time after buying LoRa stuff for fun and experiments, I finally took a look at it.

LoRa is a wireless communication protocol. It is designed to be power-efficient and allow long range (as its name, LOng RAnge) communications.

Like many other things in tech, a beginner in LoRa will have to browse from incomplete and outdated blog posts, to extremely detailed vendor's provided hardware specs without a single line of code.

I found some good guides about LoRa and thought I would make a small blog post about it.

I got in my hand two flavour of LoRa : 
- a LoRa HAT module (868 MHz) (ref: SKU22571), designed to work with a Raspberry Pi
- a E32-433T30D (433 MHz) (ref: SX1278), designed to work with Arduino


### Raspberry Pi

Based on doc and waveshare's repo, wrote my own implementation


Here's my repo : [https://github.com/20100dbg/lora](https://github.com/20100dbg/lora)

### Arduino


- common settings
- transparent transmission (using 65535)
- fixed point transmission
- repeater
- security
