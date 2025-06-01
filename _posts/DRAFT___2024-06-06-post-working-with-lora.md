---
title: "Working with LoRa"
categories:
  - Hardware
tags:
  - LoRa
  - Raspberry Pi
  - Arduino
  - ESP32
  - STM32
---

LoRa is a wireless communication protocol. It is designed to be power-efficient and allow long range (as its name, LOng RAnge) communications.

Like many other things in tech, a beginner in LoRa will have to browse from incomplete and outdated blog posts, to extremely detailed vendor's provided hardware specs without a single line of code.




I got a few different flavours of LoRa : 
- LoRa HAT module (868 MHz) , designed to work with a Raspberry Pi /// https://learn.sb-components.co.uk/LoRa-HAT-for-Raspberry-Pi
- Lo-Fi https://github.com/sbcshop/Lo-Fi_Software
- E32-433T30D (433 MHz) /// https://www.fr-ebyte.com/products/E32-433T30D/1
- Lora Wio e5 mini https://wiki.seeedstudio.com/LoRa_E5_mini/



### LoRa HAT module

based on ebyte ...
find the datasheet


Here's my repo : [https://github.com/20100dbg/lora](https://github.com/20100dbg/lora)


### What I learned on LoRa


About adresses : Using transparent transmssion mode (default), your local address MUST match your recipient one : if A is sending a message to B, A must have B's address and network (at least temporarly) We can use special address 65535 that allow broadcast (send to everyone) and monitoring (listen from everyone). A broadcast message is sent to any address and any network.

So unless I have a particular need, I setup every node with 65535 address, and add a logical address in my homemade data header.

Using fixed point transmission mode :

- Each node has a unique address
- Nodes must be on the same network, or send message through repeater
- Nodes can use the same channel or different ones.
- Each message MUST start with three bytes describing your recipient as : ADDRH+ADDRL+CHANNEL
- If your recipient is on another channel, your node is hopping briefly on recipient's channel to send the message

About RSSI (Received Signal Strength Indication) : There is two modes

- RSSI ambient noise (aka Channel noise) (register 04) : gives only on demand current channel noise and/or last message's signal level
- RSSI (register 06) : the chip append automatically to each message received a byte with RSSI value

We can activate none, one, or both modes. Note : real_rssi = -1 * (256 - RSSI byte value)

About repeater mode

- Need nodes to use fixed point modes (maybe not mandatory but recommended)
- Nodes need to share the same address and different networks
- On clients, network must be same value than channel
- Since we are in fixed point mode, each message start with three bytes : ADDRH+ADDRL+CHANNEL
- Repeater must set its ADDRH as a first network and ADDRL as a second network. No need to set a network on repeater.
- Repeater can only listen one channel at time, so choose it wisely
- Unless you switch channel regularly, message will travel in one way only

File transfer : It seems really difficult without a TCP like protocol

Packets MAY be arbitrarly split by the chip


### What I learned on serial communications

when reading input buffer, the buffer may not be full; the buffer is being filled byte by byte, and if you are unlucky

