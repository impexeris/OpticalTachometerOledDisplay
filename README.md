# OpticalTachometerOledDisplay

Arduino Pro Micro-based Optical Tachometer with OLED display, Infrared Emitter and IR Detector Phototransistor Pair TCRT5000 Range Sensor and optical signal preamplifier for a chinise 300W 12000RPM ER11 Brushed Air-Cooled DC 48V Spindle Motor. It uses an TCRT5000 IR LED and IR PhotoTransistor pair to detect motor fan blades or the ER11 motor,  2N3904 transistor apmlifying stage and Arduino Pro Mini (an Atmega32u4 based board) to calculate and display the RPM.
here are videos of a test setup here:
https://youtu.be/UY7F-yI5QF4 ( this is the video with the analog gauge scale remade from 0 to 15000RPM to use the analog gauge more efficiently)
https://youtu.be/u5U58VDxa34 (this is a video with an original analog gauge scale which is from 0 to 30000RPM)
https://youtu.be/Aiq3dz5xzpg) (this video shows the output signal of the detector amplifier which is passed to the micro-controller)

Instructable on building the Sensor and connecting it to the Arduino (work in progress to be completed soon)

### Prerequisites
Your favorite Arduino IDE.

You'll also need the Adafruit Adafruit_SSD1306 and the Adafruit-GFX-Library. Both libraries are free and the links are listed below. 
https://github.com/adafruit/Adafruit_SSD1306
https://learn.adafruit.com/adafruit-gfx-graphics-library/overview

Adafruit Tutorial on adding libraries to the Arduino IDE
https://learn.adafruit.com/arduino-tips-tricks-and-techniques/arduino-libraries

## Authors

* **Troy Barbour** - *Initial work* - [tmbarbour](https://github.com/tmbarbour)
* **impexeris** - *detector/amplifier* *adaptation to run on Arduino PRO Micro (Atmega32u4)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* The entire code is from Troy Babour original with a slight modification to match detector, reduced analog gauge range and added delay to prevent flickering of the display during the switching on of the circuit .  

