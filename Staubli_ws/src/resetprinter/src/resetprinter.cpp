#include <iostream>
#include <fstream>
#include "mySerial.h"
// Core ros functionality like ros::init
// #include <ros/ros.h>


// This program is just opening the file, and sending it to the USB line by line.


int main(int argc, char** argv)
{
    // Initialize ROS
    // ros::init(argc, argv, "resetprinter");
    // ros::NodeHandle nh;

    std::string filename, line, path;
    std::ifstream read;

    // Define port and baudrate, and make communication.
    mySerial serial = mySerial("/dev/ttyUSB0", 9600);

    // UNCOMMENT THIS, IF you want your file to be read from same directory with the script.
    /*
    std::cout << "Type ""reset.txt""if you want to reset the printer: ";
    std::getline(std::cin, filename);
    read.open(filename);
    */
   
    // UNCOMMENT THIS, IF you want your file to be read from absolute path.
    
    std::cout << "Type resetprinter.gcode if you want to reset the printer: ";
    std::getline(std::cin, filename);
    read.open("/home/tomiliatsereh/backupstaubli_ws/src/manufacturing_6dof/gcode/" + filename); // "home/[USERNAME]/[PATH_TO_YOUR_FILE]" <- edit
    

    // UNCOMMENT THIS, IF you want to ask user for path and filename.
    /*
    std::cout << "Enter full path to the file location: ";
    std::getline(std::cin, path);

    std::cout << "Enter G-Code filename: ";
    std::getline(std::cin, filename);
    
    read.open(path + filename);
    */


    if (read.is_open())
    {
        while (getline(read, line))
        {
            // cout just to see what is happening
            std::cout << line << "\n";
            // sending our gcode line by line
            serial.Send(line);
        }
        read.close();
    }

    else
        std::cout << "Unable to open file \n";

    return 0;
}