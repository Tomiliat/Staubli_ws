#include <iostream>
#include <fstream>
#include "mySerial.h"
#include <math.h>
#include <cmath>
#include <sstream>
#include <vector>
// Core ros functionality like ros::init
#include <ros/ros.h>
//The tf package provides an implementation of a TransformListener to help make the task of receiving transforms easier. 
//To use the TransformListener, we need to include the tf/transform_listener.h header file.
#include <tf/transform_listener.h>

// printer variables & parameters (F)
float printspeed = 40;      // Set velocity 
float diambuse = 0.4;       // nozzle
float hcouche = 0.2;        //layer
float diamfil = 1.75;       //filament diameter
// F = feedrate = (60 * 4 * diambuse * hcouche * printspeed)/(diamfil^2 * pi)
// E = ((pi * diamfil^2) / (4)) * L1. 

// Function for searching coordinates.
std::string getGcodeValue(std::string gcodeOperand, std::string gcodeLine) {
    if (isdigit(gcodeLine[gcodeLine.find(gcodeOperand) + 1]) || gcodeLine[gcodeLine.find(gcodeOperand) + 1]=='-')
    {
        int valueStartPos = gcodeLine.find(gcodeOperand) + 1;
        int spacePos = gcodeLine.find(" ", valueStartPos);
        int valueLength;
        std::string gcodeValue;
        if (spacePos == std::string::npos)
        {
            // Space not found after operand, value ends at the line end
            valueLength = gcodeLine.length() - valueStartPos;
            gcodeValue = gcodeLine.substr(valueStartPos);
        }
        else
        {
            // After operand found space, value is between operand and " "
            valueLength = spacePos - valueStartPos;
            gcodeValue = gcodeLine.substr(valueStartPos, valueLength);
        }
        return gcodeValue;
    }
    return "";
}


int main(int argc, char** argv)
{
    // Initialize ROS
    ros::init(argc, argv, "printertool");
    ros::NodeHandle nh;

    // Calculate new parameters to F
    float feedrate = (60 * 4 * diambuse * hcouche * printspeed)/(std::pow(diamfil, 2) * M_PI);

    std::ostringstream ssF;
    ssF << feedrate;
    std::string feedrateString = ssF.str();

    std::string filename, line, path;
    std::ifstream read;

    // Define port and baudrate, and make communication.
    mySerial serial = mySerial("/dev/ttyUSB0", 9600);

    std::vector<std::string> gcodeLines = {};

    // UNCOMMENT THIS, IF you want your file to be read from same directory with the script. ///////////////////////
    /*                                                                                                              //
    std::cout << "Enter G-Code filename: ";                                                                         //
    std::getline(std::cin, filename);                                                                               //
    read.open(filename);                                                                                            //
    */                                                                                                              //
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   

    // UNCOMMENT THIS, IF you want your file to be read from absolute path. ///////////////////////////////////////////////////////////////////////
    /*                                                                                                                                           //  
    std::cout << "Enter G-Code filename: ";                                                                                                       //  
    std::getline(std::cin, filename);                                                                                                             //
    read.open("/home/tomiliatsereh/backupstaubli_ws/src/manufacturing_6dof/gcode/" + filename); // "home/[USERNAME]/[PATH_TO_YOUR_FILE]" <- edit  //
    */                                                                                                                                              //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //* UNCOMMENT THIS, IF you want to ask user for path and filename. ///////////////////////////////////////////////
    //*                                                                                                               //
    std::cout << "Enter full path to G-code file folder: ";                                                          //
    std::getline(std::cin, path);                                                                                    //
                                                                                                                     //
    std::cout << "Enter G-Code filename: ";                                                                          //
    std::getline(std::cin, filename);                                                                                //
                                                                                                                     //
    read.open(path + filename);                                                                                      //
    //*/                                                                                                               //
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    if (read.is_open())
    {
        std::string oldE = "0";
        std::string actE;
        double newE;

        // Search F and change feedrate value. Overwrite with new parameters.
        while (getline(read, line))
        {
            if (isdigit(line[line.find("F") + 1]))
            {
                int fStartPos = line.find("F") + 1;
                int fSpacePos = line.find(" ", fStartPos);
                int feedrateLength = fSpacePos - fStartPos;
                line.replace(fStartPos, feedrateLength, feedrateString);
            }
            /*
            // There is possibility, that F is not stored and should be sended in every line. 
            else
            {
                If in future there occurs problems with printing, start with adding F in every line. This could be the main point to start
                figuring out what is wrong. Second point is to look at G0 and G1. If this will not work, other way could be to send to the printer
                lines in shape like only "G0 E0.123 F12.345". Remember, that in Gcode E parameter is growing all the time, so it should be calculated 
                for only one point.
            }
            */


            // Search E and change parameters
            if (isdigit(line[line.find("E") + 1]))
            {
                int eStartPos = line.find("E") + 1;
                int eSpacePos = line.find(" ", eStartPos);
                int eLength;
                if (eSpacePos == std::string::npos)
                {
                    // Space not found after "E", extruder value ends at the line end
                    eLength = line.length() - eStartPos;
                    actE = line.substr(eStartPos);
                }
                else
                {
                    // After "E" found space, extruder value is between "E" and " "
                    eLength = eSpacePos - eStartPos;
                    actE = line.substr(eStartPos, eLength);
                }
                newE = std::stod(actE) - std::stod(oldE);
                // Convert newE to string
                std::ostringstream ssE;
                ssE << newE;
                std::string extruderString = ssE.str();
                line.replace(eStartPos, eLength, extruderString);
                oldE = actE;
            }

            // cout just to see what is happening
            // std::cout << line << "\n";

            gcodeLines.push_back(line);
        }
        read.close();

        /*
        Create a TransformListener object. Once the listener is created, it starts receiving tf transformations over the wire, and buffers them for up to 50 seconds. 
        The TransformListener object should be scoped to persist otherwise it's cache will be unable to fill and almost every query will fail. A common method is to make the 
        TransformListener object a member variable of a class.
        */

        tf::TransformListener listener;
        ros::Rate rate(50.0);   //  frequency (hz)
        int lineIndex = 0;
        
        // Robot start position coordinates
        double destinationX = 0;
        double destinationY = 0;
        double destinationZ = 0;

        // Sending all lines at the beginning, before first line with XY AND Z is found. After line with XYZ is found, waiting coordinates to match with the robot and sending them line by line.
        // When robot reaches point A, send point B to the USB etc.
        // If line in gcode does not include XY or Z, send it straightly to the USB. 
        while (destinationX==0 && destinationY==0 && destinationZ==0)
        {
            // Sending lines to USB port. When XYZ found, stop and wait for matching coordinates from robot.
            if (getGcodeValue("X", gcodeLines[lineIndex])!="" && getGcodeValue("Y", gcodeLines[lineIndex])!="" && getGcodeValue("Z", gcodeLines[lineIndex])!="")
            {
                destinationX = std::stod(getGcodeValue("X", gcodeLines[lineIndex]));
                destinationY = std::stod(getGcodeValue("Y", gcodeLines[lineIndex]));
                destinationZ = std::stod(getGcodeValue("Z", gcodeLines[lineIndex]));
            }
            std::cout << gcodeLines[lineIndex] << "\n";
            serial.Send(gcodeLines[lineIndex]);
            lineIndex++;
        }
        while (nh.ok()){
            tf::StampedTransform transform;
            try{
                /*
                The waitForTransform() takes four arguments:
                1. Wait for the transform from this frame...
                2. ... to this frame,
                3. at this time, and
                4. timeout: don't wait for longer than this maximum duration.
                So waitForTransform() will actually block until the transform between the frames becomes available (this will usually take a few milliseconds),
                OR --if the transform does not become available-- until the timeout has been reached.
                */
                listener.waitForTransform("custom","tool0", ros::Time(0), ros::Duration(10.0));

                /*
                Here, the real work is done, we query the listener for a specific transformation. Let's take a look at the four arguments:
                1. We want the transform from frame 1 to frame 2.
                2. The time at which we want to transform. Providing ros::Time(0) will just get us the latest available transform.
                3. The object in which we store the resulting transform.
                We can also use ros::Time::now(), but for real tf use cases, it is often perfectly fine to use Time(0).
                */
                listener.lookupTransform("custom", "tool0", ros::Time(0), transform);

                // Latest XYZ transform
                double x = transform.getOrigin().x();
                double y = transform.getOrigin().y();
                double z = transform.getOrigin().z();

                // Print robot location to the console
                // std::cout << "Current position: (" << x << "," << y << "," << z << ")" << std::endl;

                // Checking and waiting matching values, from gcode file and robot position.
                // std::cout << (x*1000) << " == " << destinationX << "\n";
                // std::cout << (y*1000) << " == " << destinationY << "\n";
                // std::cout << (z*1000) << " == " << destinationZ << "\n";

                // Set tolerance between points. Decrease value for better accuracy. 
                double tolerance = 0.05;
                if (std::abs(x*1000-destinationX)<tolerance && std::abs(y*1000-destinationY)<tolerance && std::abs(z*1000-destinationZ)<tolerance)
                {
                    // Send line, if it is not including XYZ straight away
                    while (getGcodeValue("X", gcodeLines[lineIndex])=="" && getGcodeValue("Y", gcodeLines[lineIndex])=="" && getGcodeValue("Z", gcodeLines[lineIndex])=="")
                    {
                        std::cout << gcodeLines[lineIndex] << "\n";
                        serial.Send(gcodeLines[lineIndex]);
                        lineIndex++;
                    }
                     // XYZ found, send line and save coordinate values.
                    std::cout << gcodeLines[lineIndex] << "\n";
                    serial.Send(gcodeLines[lineIndex]);

                    if (getGcodeValue("X", gcodeLines[lineIndex])!="")
                    {
                        destinationX = std::stod(getGcodeValue("X", gcodeLines[lineIndex]));
                    }
                    if (getGcodeValue("Y", gcodeLines[lineIndex])!="")
                    {
                        destinationY = std::stod(getGcodeValue("Y", gcodeLines[lineIndex]));
                    }
                    if (getGcodeValue("Z", gcodeLines[lineIndex])!="")
                    {
                        destinationZ = std::stod(getGcodeValue("Z", gcodeLines[lineIndex]));
                    }

                    lineIndex++;
                    std::cout << gcodeLines[lineIndex] << "\n";
                }
            }
            catch (tf::TransformException &ex) {
                ROS_ERROR("%s",ex.what());
                ros::Duration(1.0).sleep();
                
                continue;
            }

            rate.sleep();
        }
    }

    else
        std::cout << "Unable to open file \n";

    return 0;
}