ROS 3d printer project. 
02.02.2020-31.5.2020

This package is for the printer part of the project. Physical 3D Printer is built in common way, using Arduino MEGA 2560, Ramps 1.4 card and inside it is running Marlin software.

Shortly: printertool.cpp is sending G-code to the usb port, one line at the time. MySerial packages handle connection to the port.

Currently printertool node is asking for the path to the file, but this could be changed in printertool.cpp. In the program is possibility to define path, and afterwards it will go straightly to the folder with the file, but the path changes for every user. For that reason it was set to ask path, to avoid misleading.
