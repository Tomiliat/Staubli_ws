All tests were performed by driving staubli at first to the 0 point of a redefined frame, and starting from here for more clear information. Test file was frametest.gcode with following points:
X0 Y0 Z0 I0.0 J0.0 K1.0
X50 Y0 Z0 I0 J0 K1.0
X0 Y50 Z0 I0.0 J0.0 K1.0
X0 Y0 Z0 I0.0 J0.0 K1.0

Note: Controller is set to 50%

As could be seen, when trajectory is sent via descartes, changing velocity parameters are affecting to the information it is sending in both cases, with and without controller.

RViZ:
When running descartes only in simulation view (without connection to the physical robot), velocity changes are visible, but when controller is connected, velocity changes dissapear and simulation view is showing exactly how the robot moves in real life, without affection of velocity changes in descartes. With this explanation, i'm trying to say, that it is not the case that simulation view is going by it's own velocity parameters, and real robot is going by it's own. It changes in simulation and is showing exactly how physical robot is acting.

Could be MoveIT configuration file. This is just a bet and could be not related at all.
http://docs.ros.org/kinetic/api/moveit_tutorials/html/doc/time_parameterization/time_parameterization_tutorial.html

Or ros conrol?

Maybe this application is/is not specified in ros control?

"If you are using a position controller, it will move to the commanded position as fast as possible (max velocity). If you want it to move slower than that, you can 1) set the velocity limit to be lower, 2) send incremental position commands that will ensure it arrives at the speed you want, or 3) use the velocity controller instead."
https://answers.ros.org/question/261939/how-to-control-a-joint-with-position-and-velocity-using-ros_control/
