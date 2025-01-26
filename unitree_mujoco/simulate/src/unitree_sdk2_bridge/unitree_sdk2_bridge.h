#ifndef UNITREE_SDK2_BRIDGE_H
#define UNITREE_SDK2_BRIDGE_H

#include <iostream>
#include <chrono>
#include <cstring>

#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <unitree/idl/go2/WirelessController_.hpp>
#include <unitree/idl/go2/LowState_.hpp>
#include <unitree/idl/go2/LowCmd_.hpp>
#include <unitree/idl/hg/LowCmd_.hpp>
#include <unitree/idl/hg/LowState_.hpp>
#include <mujoco/mujoco.h>

#include "../mujoco_pcl/pointCloudGenerator.hpp"


using namespace unitree::common;
using namespace unitree::robot;
using namespace std;

#define TOPIC_LOWSTATE "rt/lowstate"
#define TOPIC_HIGHSTATE "rt/sportmodestate"
#define TOPIC_LOWCMD "rt/lowcmd"
#define TOPIC_PCL "rt/pointcloud2"

#define MOTOR_SENSOR_NUM 3
#define NUM_MOTOR_IDL_GO 20
#define NUM_MOTOR_IDL_HG 35


class UnitreeSdk2Bridge
{
public:
    UnitreeSdk2Bridge(mjModel *model, mjData *data);
    ~UnitreeSdk2Bridge();

    void LowCmdGoHandler(const void *msg);
    void LowCmdHgHandler(const void *msg);

    void PublishLowStateGo();
    void PublishLowStateHg();
    void PublishHighState();
    void Run();
    void PrintSceneInformation();
    void CheckSensor();
    void PublishPointCloud();

    ChannelSubscriberPtr<unitree_go::msg::dds_::LowCmd_> low_cmd_go_suber_;
    ChannelSubscriberPtr<unitree_hg::msg::dds_::LowCmd_> low_cmd_hg_suber_;

    unitree_go::msg::dds_::LowState_ low_state_go_{};
    unitree_hg::msg::dds_::LowState_ low_state_hg_{};
    unitree_go::msg::dds_::SportModeState_ high_state_{};

    ChannelPublisherPtr<unitree_go::msg::dds_::LowState_> low_state_go_puber_;
    ChannelPublisherPtr<unitree_hg::msg::dds_::LowState_> low_state_hg_puber_;
    ChannelPublisherPtr<unitree_go::msg::dds_::SportModeState_> high_state_puber_;
    ChannelPublisherPtr<sensor_msgs::msg::dds_::PointCloud2_> pcl_puber_;
   
    ThreadPtr lowStatePuberThreadPtr;
    ThreadPtr HighStatePuberThreadPtr;
    ThreadPtr PointCloudPuberThreadPtr;

    mjData *mj_data_;
    mjModel *mj_model_;
    mjrContext *context_;

    int num_motor_ = 0;
    int dim_motor_sensor_ = 0;

    int have_imu_ = false;
    int have_frame_sensor_ = false;
    int idl_type_ = 0; // 0: unitree_go, 1: unitree_hg

    PointCloudGenerator *pcl_generator_;
    bool camSet_ = false;
    std::mutex mut_;
    int camera_id_;
    mjrRect viewport_ = {0,0,100,100}; // Same as the camera's resolution

};

#endif
