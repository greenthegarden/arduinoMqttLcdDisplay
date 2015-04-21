# arduinoMqttLcdDisplay

Display data published by a MQTT broker on a 16x2 LCD Display connected to an Arduino with Ethernet connection.

The configuration I use is a [Freetronics Etherten] (http://www.freetronics.com.au/collections/ethernet/products/etherten) board connected to a [Freetronics LCD & Keypad Shield] (http://www.freetronics.com.au/products/lcd-keypad-shield).

The following libraries are required which are not included with the Arduino IDE:

* Time from https://www.pjrc.com/teensy/td_libs_Time.html
* PubSubClient from http://knolleary.net/arduino-client-for-mqtt/

The display currently offers two menus. The first 'display' shows the time and date (determined from an ntp server) and ip address. The second, 'weather', displays the current temperature and humidity as reported from a [Sparkfun Weather Board via MQTT] (https://github.com/greenthegarden/MqttWiFlyWeatherBoard).
