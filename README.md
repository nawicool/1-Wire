#   1-Wire DS18B20 program



1. The first programm DS18B20.c is arduino program allows to communicate with DS18B20 Sensor in parasite power mode.
Sensor is connected  with arduino pin2  & allows the communication over usart.DS18B20(new).c contains some non ardunio functions. 


2. The second programm DS18B20Multiple.c allows the communication with multiple sensors. (Also uses pin2)
You can find multiple devices on the bus. You just read the adresses. 0 is dominant. If you find a device that has a dominant bit then the other device with non dominant bit  doesn't answer any more & let the device with dominant bit to answer.
