#!/bin/bash
echo "Setup unitree ros2 simulation environment"
source /opt/ros/jazzy/setup.bash
source $HOME/unitree_proj/unitree_ros2/cyclonedds_ws/install/setup.bash*
export ROS_DOMAIN_ID=1
export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
export CYCLONEDDS_URI='<CycloneDDS><Domain><General><Interfaces>
                            <NetworkInterface name="lo" priority="default" multicast="default" />
                        </Interfaces></General></Domain></CycloneDDS>'


