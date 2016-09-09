# Problem definition
- HOBO datalogger used widely; YSI
- Costs
- Want something extensible
- Something that can connect to the hobby electronics movement
- Use existing code with minimal fuss / muss
- Wanted to make it easy to deploy -- pvc, water bottle, some easy and cheap enclosure
- Easy to share data -- sd card

# Form factor and enclosure
- Wanted to enable water bottle as ‘easiest option’
- Also allows for thin PVC

https://publiclab.org/notes/mathew/02-26-2015/sensors-in-soda-bottles
https://publiclab.org/notes/mathew/01-16-2015/sketching-a-waterproof-pop-bottle-sensor-system

<img src="pics/bottle_enclosure.png" width=300>

<img src="pics/rice.jpg" width=300> 

<img src="pics/rubber_stopper.jpg" width=300> 

<img src="pics/screws_a.jpg" width=300> 

<img src="pics/screws_b.jpg" width=300> 

<img src="pics/pvc.jpg" width=300> 

<img src="pics/bottle.png" width=300>


# What drove IC / placement choices
- Atmel vs ARM
- CH340 -- cheap and easy (but not easy to source)
- DS3231 -- want temp-compensated RTC to line up timestamps
- 2x7 header at end -- to allow water bottle config
- SPI and I2C breakouts at end 
- Hardware interrupts D2 and D3 for any related applications (freq-based measurements like conductivity and depth)
- EEPROM for storing configurations
- Mounting holes for gen purpose mounting

# Power management / choices
- Battery charging circuit
- But requires protective diode to allow for disconnect from USB Serial chip
- So, want to avoid voltage drop
- So, create “gen” battery input, without diode / voltage drop
- RTC alarm functionality 
- Want very low power -- mosfet on external board, mosfet on SD card, mosfet on battery measurement

