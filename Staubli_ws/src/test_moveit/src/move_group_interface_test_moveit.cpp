#include <ros/ros.h>
#include <moveit/move_group_interface/move_group_interface.h> 
#include <moveit/planning_scene_interface/planning_scene_interface.h> //not used in this example



int main(int argc, char **argv) { 
    // ROS initialization
    ros::init(argc, argv, "move_group_interface_test_moveit"); 
    ros::NodeHandle node_handle;   
    ros::AsyncSpinner spinner(1); 
    spinner.start(); 
  
    // Move group setup
    static const std::string PLANNING_GROUP = "manipulator";
    moveit::planning_interface::MoveGroupInterface move_group(PLANNING_GROUP); 
    const robot_state::JointModelGroup* joint_model_group = move_group.getCurrentState()->getJointModelGroup(PLANNING_GROUP);
    move_group.setPlannerId("RRTConnect");
    move_group.setMaxVelocityScalingFactor(1.0);
    moveit::planning_interface::MoveGroupInterface::Plan my_plan;
    //moveit::planning_interface::PlanningSceneInterface planning_scene_interface;
  
    
    // Target joint-space home
    std::vector<double> joint_group_home{0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    
    // Target pose home with quaternions
    geometry_msgs::Pose target_home;
    target_home.position.x = 0.0;
    target_home.position.y = 0.035;
    target_home.position.z = 0.835;
    target_home.orientation.x = 0.0;
    target_home.orientation.y = 0.0;
    target_home.orientation.z = 0.0;
    target_home.orientation.w = 1.0;
    
    // Target pose for the end-effector (random valid)
    geometry_msgs::Pose target_pose1;
    target_pose1.position.x = -0.286879;
    target_pose1.position.y = -0.00978807;
    target_pose1.position.z = 0.320535;
    target_pose1.orientation.x = -0.448129;
    target_pose1.orientation.y = -0.877071;
    target_pose1.orientation.z = 0.100327;
    target_pose1.orientation.w = 0.14093;
    
    // Plan and move to pose1
    move_group.setPoseTarget(target_pose1);
    move_group.move();

        
    // Cartesian path
    std::vector<geometry_msgs::Pose> waypoints;
    geometry_msgs::Pose target_pose_next = target_pose1;
    waypoints.push_back(target_pose_next);

    target_pose_next.position.z += 0.2;
    waypoints.push_back(target_pose_next);

    target_pose_next.position.y += 0.1;
    waypoints.push_back(target_pose_next);

    target_pose_next = target_pose1;
    waypoints.push_back(target_pose_next);
    
    //Cartesian path parameters
    moveit_msgs::RobotTrajectory trajectory;
    const double eef_step = 0.01;
    const double jump_threshold = 0.0;
    
    // Plan and move the trajectory
    move_group.computeCartesianPath(waypoints, eef_step, jump_threshold, trajectory);
    my_plan.trajectory_ = trajectory;
    move_group.execute(my_plan);
    

    // Plan and move to home (joints values)
    move_group.setJointValueTarget(joint_group_home);
    move_group.move();
    
    
    //Get pose
    geometry_msgs::PoseStamped robot_pose;
    robot_pose = move_group.getCurrentPose();
    ROS_INFO_STREAM("Actual pose:  " << robot_pose );
    

    //end
    ros::shutdown();   
} 
