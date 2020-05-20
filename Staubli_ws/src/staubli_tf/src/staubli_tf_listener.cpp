#include <ros/ros.h>
//The tf package provides an implementation of a TransformListener to help make the task of receiving transforms easier. 
//To use the TransformListener, we need to include the tf/transform_listener.h header file.
#include <tf/transform_listener.h>


int main(int argc, char** argv){
  ros::init(argc, argv, "my_tf_listener");

  ros::NodeHandle node;

  /*
  Create a TransformListener object. Once the listener is created, it starts receiving tf transformations over the wire, and buffers them for up to 10 seconds. 
  The TransformListener object should be scoped to persist otherwise it's cache will be unable to fill and almost every query will fail. A common method is to make the 
  TransformListener object a member variable of a class.
  */
  tf::TransformListener listener;

  ros::Rate rate(50.0);
  while (node.ok()){
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
      listener.waitForTransform("base_link","tool0", ros::Time(0), ros::Duration(10.0));

      // Using this also helps to figure out what printer is reading. Useful for bug fixes.
      // listener.waitForTransform("custom","tool0", ros::Time(0), ros::Duration(10.0));

      /*
      Here, the real work is done, we query the listener for a specific transformation. Let's take a look at the four arguments:
      1. We want the transform from frame 1 to frame 2.
      2. The time at which we want to transform. Providing ros::Time(0) will just get us the latest available transform.
      3. The object in which we store the resulting transform.
      We can also use ros::Time::now(), but for real tf use cases, it is often perfectly fine to use Time(0).
      */
      listener.lookupTransform("base_link", "tool0", ros::Time(0), transform);

      // As previously mentioned, for printertool bug fixes.
      // listener.lookupTransform("custom", "tool0", ros::Time(0), transform);
      

      // Print location to the console
       double x = transform.getOrigin().x();
       double y = transform.getOrigin().y();
       double z = transform.getOrigin().z();
       double rx = transform.getRotation().x();
       double ry = transform.getRotation().y();
       double rz = transform.getRotation().z();

       std::cout << "Current position: (" " X" << (x*1000) << " Y" << (y*1000) << " Z" << (z*1000) << " Rx" << (rx*1000) << " Ry" << (ry*1000) << " Rz" << (rz*1000) << ")" << std::endl;
       
    }
    catch (tf::TransformException &ex) {
      ROS_ERROR("%s",ex.what());
      ros::Duration(1.0).sleep();
      
      continue;
    }

    rate.sleep();
  }
  return 0;
};