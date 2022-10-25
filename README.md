# PhaseVortexGen_iSCAT

## Description
Acousto-Optic deflectors deflect part of the laser beam using vibrating piezo crystals under voltage. Where the difference in voltage frequency changes the deflected angle and amplitude changes the intensity of the deflected beam.

By periodically deflecting green laser onto a ruby plate, in such a way that the laser scans through a square while changing amplitude according to the corresponding pixel of some image, we create a heat signature on the plate. 

Heated parts of the plate then have different refraction indeces shifting the phase of the red laser passing through, thus creating a phase vortex.

![experiment](chess.png)

## Setup
Arduino connects to two [Direct digital synthesisers (DDS)](http://www.aaoptoelectronic.com/our-products/dds-direct-digital-synthesizers/) controlling the output voltage frequency and amplitude. DDSs then connect to corresponding Acousto-Optic deflectors.

As the speed of the sampling rate is crucial to prevent cool down of the ruby plate, we chose to use [Arduino Due](https://cs.wikipedia.org/wiki/Arduino_Due) for its high cpu frequency of 84MHz.

![Setup](optics_setup.png)

## Usage

By running program "convert.exe", you will be asked to write image number, path to the image and the output image resolution. Then a file called image<number>.h gets created, as well as some other files that exist for debugging purposes, according to the settings set in the "settings.txt" file.
  
Program "amp_control_fast.ino" includes a file image1.h and draws it using the deflectors.
  
Program "amp_control_multiple-imgs.ino" includes files image<number>.h, where number goes from 1 to 9, and draws it using the deflectors. You can change the image being drawn by selecting a corresponding number on the keypad connected to arduino.
  
## Settings
