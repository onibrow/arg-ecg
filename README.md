[Eval Board](http://www.analog.com/media/en/technical-documentation/user-guides/UG-426.pdf)

[ADAS 1000](http://www.analog.com/media/en/technical-documentation/data-sheets/ADAS1000_1000-1_1000-2.pdf)

![patient](patient_cable_pinout.jpg)

There are two ADAS's on the board, one master one slave for a total of 12 electrode input. We'll utilize only 5 of them from the master chip using the pins LA, RA, LL, RLD, V1, V2. RLD (Right Leg Driver) can also be used as the common mode electrode. We can choose different electrode setups after we get the hardware to work by toggling what we want to use and get the optimal setup

Pins to use from the Patient Cable: 7, 9, 10, 11, 14

If we break the D-Bus just find another from Digikey

![trouble](trouble_shooting.png)

![unused](unused_pins.png)

9/12 testing: We got the 5 electrode ECG to work with the software. LA, RA, LL were getting good signals when using alongside RLD and CE. We tried removing RLD for fewer electrodes and turns out the default setting uses RLD as the reference drive (pg 10 Table 5) so we cannot do that until we mess with the register settings.

ADAS1000 Example Schematic: http://www.analog.com/media/en/reference-design-documentation/reference-designs/CN0308.pdf

## Flashing

There is a good chip that supposedly integrated many of the function we need on a single module: the [CC3200MOD](http://www.ti.com/product/CC3200MOD/samplebuy). [Here](https://e2e.ti.com/support/wireless_connectivity/simplelink_wifi_cc31xx_cc32xx/f/968/t/514171?Questions-about-how-to-program-a-CC3200-based-custom-board) is a forumn post that answers how to flash the chip.
