# Hardware
- Capacitors -- George
- Switch functionality
- Mislabeled rows on protoboard
- SD cards are unreliable re: low power operation (E Mallon, links)
- Sleeping current could be lower
- Perhaps drop battery charging altogether
- CH340 chip isn’t easy to source -- switch to FTDI cable, or another processor (M0)

# Software
- RTC functionality -- was ‘sleeping’ at high current -- SD card seemed responsible
- Kina wrote RTC ‘interrupt’ code -- now sleeps at much lower current
- Code for logging to SD card -- improvement brought about by the switch functionality

