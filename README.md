# smart_watch_round_display
Smart Clock with 1,28" display with integrated ESP32-C3
(ESP32-C3 with integrated GC9A01)


So, i looked for a clock for my sleeping room. It should be bright enough to have a clear view und dark enough, that sleeping is no problem. As i found nothing in the worl wide shopping mall, i try to build one by myself.

You only need a Module with a round display and integrated module. I ordered one at the big company in china. 

So the thing is really easy, you need a programm to compile the sketch and then transfer it to the device. I personally use Arduino IDE.

# What does the sketch do ?

It connects to your Wifi-Network and after that, to your MQTT Broker. (I think about if its a good idea to see the ouside tempeature on the display). After that, it gets the actual Time from the network and a red dot beginns to run around the display. 
So the red dot is the "second" and the time is displayed in the middle. The temperature is displayed under the temperature.

So the only think you need is a housing for the small devices and some changes for the code (Wifi, MQTT). Thats is.


# Thanks to:

https://werner.rothschopf.net/microcontroller/202103_arduino_esp32_ntp_en.htm

https://github.com/arendst/Tasmota/discussions/19487


