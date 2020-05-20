// Core ros functionality like ros::init and spin
#include <ros/ros.h>
#include <ros/package.h>

// ROS Trajectory Action server definition
#include <control_msgs/FollowJointTrajectoryAction.h>
// Means by which we communicate with above action-server
#include <actionlib/client/simple_action_client.h>

// Includes the descartes robot model we will be using
#include <descartes_moveit/ikfast_moveit_state_adapter.h>

// Includes the descartes trajectory type we will be using
#include <descartes_trajectory/axial_symmetric_pt.h>
#include <descartes_trajectory/cart_trajectory_pt.h>

// Includes the planner we will be using
#include <descartes_planner/dense_planner.h>

// Includes the utility function for converting to trajectory_msgs::JointTrajectory's
#include <descartes_utilities/ros_conversions.h>

// Include cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <cmath>

// Boost
#include <boost/algorithm/string.hpp>

/**
 * Makes the trajectory for the robot to follow, from G-Code file
 */
std::vector<descartes_core::TrajectoryPtPtr> makePath(std::string filename);

/**
 * Sends a ROS trajectory to the robot controller
 */
bool executeTrajectory(const trajectory_msgs::JointTrajectory& trajectory);

int main(int argc, char** argv)
{
  // Initialize ROS
  ros::init(argc, argv, "descartes_manufacturing");
  ros::NodeHandle nh;

  // Since we're not calling ros::spin() and doing the planning in a callback, but rather just handling this
  // inline, we need to create an async spinner if our publishers are to work. Note that many MoveIt components
  // will also not work without an active spinner and Descartes uses moveit for its "groups" and "scene" descriptions
  ros::AsyncSpinner spinner (1);
  spinner.start();

  // 1. First thing first, let's create a kinematic model of the robot. In Descartes, this is used to do things
  // like forward kinematics (joints -> pose), inverse kinematics (pose -> many joints), and collision checking.

  // All of the existing planners (as of Nov 2017) have been designed with the idea that you have "closed form"
  // kinematics. This means that the default solvers in MoveIt (KDL) will NOT WORK WELL. I encourage you to produce
  // an ikfast model for your robot (see MoveIt tutorial) or use the OPW kinematics package if you have a spherical
  // wrist industrial robot. See http://docs.ros.org/kinetic/api/moveit_tutorials/html/doc/ikfast/ikfast_tutorial.html 

  // This package assumes that the move group you are using is pointing to an IKFast kinematics plugin in its
  // kinematics.yaml file. By default, it assumes that the underlying kinematics are from 'base_link' to 'tool0'.
  // If you have renamed these, please set the 'ikfast_base_frame' and 'ikfast_tool_frame' parameter (not in the
  // private namespace) to the base and tool frame used to generate the IKFast model.
  descartes_core::RobotModelPtr model (new descartes_moveit::IkFastMoveitStateAdapter());

  // Name of description on parameter server. Typically just "robot_description". Used to initialize
  // moveit model.
  const std::string robot_description = "robot_description";

  // name of the kinematic group you defined when running MoveitSetupAssistant. For many industrial robots this will be
  // "manipulator"
  const std::string group_name = "manipulator";

  // Name of frame in which you are expressing poses. Typically "world_frame" or "base_link".
  /* Previously we were looking at base_link, so at the middle bottom of the robot. Now there is added new frame named "custom",
     which we can move from the tx_40_macro.xacro. This is done, to be able move needed frame anywhere we want, for example
     plan is that this whill be point in the corner of the printing table. Similar changes are done to the printertool.cpp. 
     It is "looking" transformation between custom frame and tool0, so it recognizes coordinate points correctly.  
  */
  const std::string world_frame = "custom";
  // const std::string world_frame = "base_link";

  // tool center point frame (name of link associated with tool). The robot's flange is typically "tool0" but yours
  // could be anything. We typically have our tool's positive Z-axis point outward from the grinder, welder, etc.
  const std::string tcp_frame = "tool0";

  // Before you can use a model, you must call initialize. This will load robot models and sanity check the model.
  if (!model->initialize(robot_description, group_name, world_frame, tcp_frame))
  {
    ROS_INFO("Could not initialize robot model");
    return -1;
  }

  model->setCheckCollisions(true); // Let's turn on collision checking.

  // 2. The next thing to do is to generate a path for the robot to follow. The description of this path is one of the
  // cool things about Descartes. The source of this path is where this library ties into your application: it could
  // come from CAD or from surfaces that were "scanned".

  // Make the path by calling a helper function. See makePath()'s definition for more discussion about paths.
  std::string filename;
  std::cout << "Enter G-Code filename: ";
  std::getline(std::cin, filename);
  std::vector<descartes_core::TrajectoryPtPtr> points = makePath(filename);

  // 3. Now we create a planner that can fuse your kinematic world with the points you want to move the robot
  // along. There are a couple of planners now. DensePlanner is the naive, brute force approach to solving the
  // trajectory. SparsePlanner may be faster for some problems (especially very dense ones), but has recieved
  // less overall testing and evaluation.
  descartes_planner::DensePlanner planner;

  // Like the model, you also need to call initialize on the planner
  if (!planner.initialize(model))
  {
    ROS_ERROR("Failed to initialize planner");
    return -2;
  }

  // 4. Now, for the planning itself. This typically happens in two steps. First, call planPath(). This function takes
  // your input trajectory and expands it into a large kinematic "graph". Failures at this point indicate that the
  // input path may not have solutions at a given point (because of reach/collision) or has two points with no way
  // to connect them.
  if (!planner.planPath(points))
  {
    ROS_ERROR("Could not solve for a valid path");
    return -3;
  }

  // After expanding the graph, we now call 'getPath()' which searches the graph for a minimum cost path and returns
  // the result. Failures here (assuming planPath was good) indicate that your path has solutions at every waypoint
  // but constraints prevent a solution through the whole path. Usually this means a singularity is hanging out in the
  // middle of your path: the robot can solve all the points but not in the same arm configuration.
  std::vector<descartes_core::TrajectoryPtPtr> result;
  if (!planner.getPath(result))
  {
    ROS_ERROR("Could not retrieve path");
    return -4;
  }

  // 5. Translate the result into something that you can execute. In ROS land, this means that we turn the result into
  // a trajectory_msgs::JointTrajectory that's executed through a control_msgs::FollowJointTrajectoryAction. If you
  // have your own execution interface, you can get joint values out of the results in the same way.

  // get joint names - this could be from the robot model, or from the parameter server.
  std::vector<std::string> names;
  nh.getParam("controller_joint_names", names);

  // Create a JointTrajectory
  trajectory_msgs::JointTrajectory joint_solution;
  joint_solution.joint_names = names;

  // Define a default velocity. Descartes points without specified timing will use this value to limit the
  // fastest moving joint. This usually effects the first point in your path the most.
  const static double default_joint_vel = 0.5; // rad/s
  if (!descartes_utilities::toRosJointPoints(*model, result, default_joint_vel, joint_solution.points))
  {
    ROS_ERROR("Unable to convert Descartes trajectory to joint points");
    return -5;
  }

  // 6. Send the ROS trajectory to the robot for execution
  if (!executeTrajectory(joint_solution))
  {
    ROS_ERROR("Could not execute trajectory!");
    return -6;
  }
  std::cout << joint_solution << "\n";

  // Wait till user kills the process (Control-C)
  ROS_INFO("Done!");
  return 0;
}

descartes_core::TrajectoryPtPtr makeCartesianPoint(const Eigen::Isometry3d& pose, double dt)
{
  using namespace descartes_core;
  using namespace descartes_trajectory;

  return TrajectoryPtPtr( new CartTrajectoryPt( TolerancedFrame(pose), TimingConstraint(dt)) );
}

descartes_core::TrajectoryPtPtr makeTolerancedCartesianPoint(const Eigen::Isometry3d& pose, double dt)
{
  using namespace descartes_core;
  using namespace descartes_trajectory;
  return TrajectoryPtPtr( new AxialSymmetricPt(pose, M_PI / 12.0, AxialSymmetricPt::Z_AXIS, TimingConstraint(dt)) );
}


// Calculate delta time to match 40mm/min velocity. 40mm/min could change, this is test value at the moment, and printertool.cpp have the same value.
double calculateDT(Eigen::Vector3d position, Eigen::Vector3d previousPosition)
{
  // Set velocity. 0.001 = 1mm/min
  double velocity = 0.04;

  // The distance formula
  return sqrt(pow(position(0) - previousPosition(0), 2) + pow(position(1) - previousPosition(1), 2) + pow(position(2) - previousPosition(2), 2)) / velocity*60;
}

std::vector<descartes_core::TrajectoryPtPtr> makePath(std::string filename)
{
  // In Descartes, trajectories are composed of "points". Each point describes what joint positions of the robot can
  // satisfy it. You can have a "joint point" for which only a single solution is acceptable. You might have a
  // fully defined cartesian point for which many (8 or 16) different robot configurations might work. You could
  // allow extra tolerances in any of these and even more points satisfy the constraints.

  
  // Variables
  
  // First we define the folder and G-Code file from which we get the trajectory
  std::ifstream gcodeFile;
  std::string completeFilename = ros::package::getPath("manufacturing_6dof") + "/gcode/" + filename;
  // The file will be read line by line
  std::string line;
  // Each element of the line is delimited by the character 'space'
  std::string delimiters = "' '";
  // Each element is stored as a string
  std::vector<std::string> splitLine;

  // Path trajectory reurned by the function
  std::vector<descartes_core::TrajectoryPtPtr> pathTrajectory;
  // Point of the path trajectory
  descartes_core::TrajectoryPtPtr pt;
  // Initialize the pose as an identity matrix
  Eigen::Isometry3d poseN = Eigen::Isometry3d::Identity();
  // Position given by the G-Code (X,Y,Z)
  // This is also starting position (X,Y,Z)
  Eigen::Vector3d position(0.0, 0.0, 0.0);
  // Orientation given by the G-Code (I,J,K)
  // This is also starting orientation (I,J,K)
  Eigen::Vector3d toolAxisIJK(0.0, 0.0, 1.0); 
  // The free axis of the tool, corresponding to the URDF tool0 frame
  Eigen::Vector3d toolAxis = Eigen::Vector3d::UnitZ();
  // Rotation axis used to align the tools axis
  Eigen::Vector3d rotationAxis;
  // Rotation angle used to align the tools axis
  double rotationAngle;
  // The orientation of the pose
  Eigen::AngleAxisd orientationIJK;
  // If not defined, the delta time used to get to next pose
  static double dt;
  
  // Loop variable
  int i = 1;
  
  // set XY offset
  int offsetXY = 0;

  // set Z offset
  int offsetZ = 0;

  // Define "home" position, for setting right velocity between starting position and first gcode coordinate.
  // If you change home position, remember to change parameters in lines 225 & 228 too, and set .gcode end part.
  Eigen::Vector3d previousPosition(0.0, 0.0, 0.0);

  gcodeFile.open(completeFilename);
  if(gcodeFile){

    while(std::getline(gcodeFile,line)){ 
        boost::split(splitLine, line, boost::is_any_of(delimiters));
        // Loop to extract datas from each character. 
        for(std::string element : splitLine){
            // Simply add a new condition if you want to extract and store datas from another character
            if(element[0] == 'X')
                position(0) = (std::stod(&element[1])+offsetXY)/1000;
            else if(element[0] == 'Y')
                position(1) = (std::stod(&element[1])+offsetXY)/1000;
            else if(element[0] == 'Z')
                position(2) = (std::stod(&element[1])+offsetZ)/1000;
            else if(element[0] == 'I')
                toolAxisIJK(0) = std::stod(&element[1])/1000;
            else if(element[0] == 'J')
                toolAxisIJK(1) = std::stod(&element[1])/1000;
            else if(element[0] == 'K')
                toolAxisIJK(2) = std::stod(&element[1])/1000;
            else ROS_WARN("Element: %s, line: %i not handled.",
                element.c_str(), i);
        }

        
        std::cout << position << "\n";

        // Calculs to align tools axis vector
        toolAxisIJK.normalize();
        rotationAxis = toolAxis.cross(toolAxisIJK);
        rotationAngle = asin(rotationAxis.norm())+M_PI;
        orientationIJK = Eigen::AngleAxisd(rotationAngle,rotationAxis);
        
        // Homogeneous transformations
        poseN.translate(position);
        poseN.rotate(orientationIJK);
        
        // Apply velocity changes
        dt = calculateDT(position, previousPosition);
        previousPosition = position;

        // Allow degree of freedom for the tool axis
        pt = descartes_core::TrajectoryPtPtr(new descartes_trajectory::AxialSymmetricPt(poseN, M_PI / 12.0, descartes_trajectory::AxialSymmetricPt::Z_AXIS, descartes_core::TimingConstraint(dt)));
        pathTrajectory.push_back(pt);
        
        // Format for next iteration
        i++;
        splitLine.clear();
        poseN = Eigen::Isometry3d::Identity();
    }
    gcodeFile.close();
  }else{
    ROS_ERROR("G-Code file not read");
  }


  return pathTrajectory;
}

bool executeTrajectory(const trajectory_msgs::JointTrajectory& trajectory)
{
  // Create a Follow Joint Trajectory Action Client
  actionlib::SimpleActionClient<control_msgs::FollowJointTrajectoryAction> ac ("joint_trajectory_action", true);
  if (!ac.waitForServer(ros::Duration(2.0)))
  {
    ROS_ERROR("Could not connect to action server");
    return false;
  }

  control_msgs::FollowJointTrajectoryGoal goal;
  goal.trajectory = trajectory;
  goal.goal_time_tolerance = ros::Duration(1.0);
  
  return ac.sendGoalAndWait(goal) == actionlib::SimpleClientGoalState::SUCCEEDED;
}
 
