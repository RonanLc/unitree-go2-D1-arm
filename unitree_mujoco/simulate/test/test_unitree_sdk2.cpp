#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/common/time/time_tool.hpp>
#include <unitree/idl/go2/LowState_.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <unitree/idl/go2/LowCmd_.hpp>
#include <unitree/idl/ros2/PointCloud2_.hpp>
#include <time.h>

#define TOPIC_LOWSTATE "rt/lowstate"
#define TOPIC_HIGHSTATE "rt/sportmodestate"
#define TOPIC_LOWCMD "rt/lowcmd"
#define TOPIC_PCL "rt/pointcloud2"

using namespace unitree::robot;
using namespace unitree::common;


void LowStateHandler(const void *msg)
{
    const unitree_go::msg::dds_::LowState_ *s = (const unitree_go::msg::dds_::LowState_ *)msg;

    std::cout << "Quaternion: "
              << s->imu_state().quaternion()[0] << " "
              << s->imu_state().quaternion()[1] << " "
              << s->imu_state().quaternion()[2] << " "
              << s->imu_state().quaternion()[3] << " " << std::endl;
}

void HighStateHandler(const void *msg)
{
    const unitree_go::msg::dds_::SportModeState_ *s = (const unitree_go::msg::dds_::SportModeState_ *)msg;

    std::cout << "Position: "
              << s->position()[0] << " "
              << s->position()[1] << " "
              << s->position()[2] << " " << std::endl;
}


sensor_msgs::msg::dds_::PointCloud2_ PointCloudCreator(){
    sensor_msgs::msg::dds_::PointCloud2_ pcl_{};

    time_t sec;
    time(&sec);

    pcl_.header().stamp().sec() = sec;
    pcl_.header().frame_id() = "map";

    pcl_.height() = 1;
    pcl_.width() = 100; // 100 points pcl.
    pcl_.is_dense() = true; // No invalid points.

    pcl_.fields().resize(5);
    pcl_.fields()[0].name() = "x";
    pcl_.fields()[0].offset() = 0;
    pcl_.fields()[0].datatype() =  sensor_msgs::msg::dds_::PointField_Constants::FLOAT32_;
    pcl_.fields()[0].count() = 1;

    pcl_.fields()[1].name() = "y";
    pcl_.fields()[1].offset() = 4;
    pcl_.fields()[1].datatype() =  sensor_msgs::msg::dds_::PointField_Constants::FLOAT32_;
    pcl_.fields()[1].count() = 1;

    pcl_.fields()[2].name() = "z";
    pcl_.fields()[2].offset() = 8;
    pcl_.fields()[2].datatype() =  sensor_msgs::msg::dds_::PointField_Constants::FLOAT32_;
    pcl_.fields()[2].count() = 1;

    pcl_.fields()[3].name() = "rgb";
    pcl_.fields()[3].offset() = 12;
    pcl_.fields()[3].datatype() =  sensor_msgs::msg::dds_::PointField_Constants::FLOAT32_;
    pcl_.fields()[3].count() = 1;

    pcl_.fields()[4].name() = "intensity";
    pcl_.fields()[4].offset() = 16;
    pcl_.fields()[4].datatype() =  sensor_msgs::msg::dds_::PointField_Constants::FLOAT32_;
    pcl_.fields()[4].count() = 1;
    
    pcl_.point_step() = 20; // Number of bytes used by on point
    pcl_.is_bigendian() = false;
    pcl_.row_step() = pcl_.point_step() * pcl_.width();
    pcl_.data().resize(pcl_.row_step() * pcl_.height());

    
    // Filling the pointcloud
    float x_init = 0.1;
    float y_init = 0.5;
    float z_init = 1;
    uint8_t r_color = 255;
    uint8_t g_color = 100;
    uint8_t b_color = 50;
    float intensity = 2.5;
    float rgb_float = 0.0;

    //pcl_.header().stamp().nanosec() = x_init;
    
    for (int i=0; i<pcl_.width(); i++){
        memcpy(&pcl_.data()[i * pcl_.point_step() + pcl_.fields()[0].offset()], &x_init, sizeof(float));
        memcpy(&pcl_.data()[i * pcl_.point_step() + pcl_.fields()[1].offset()], &y_init, sizeof(float));
        memcpy(&pcl_.data()[i * pcl_.point_step() + pcl_.fields()[2].offset()], &z_init, sizeof(float));
        memcpy(&pcl_.data()[i * pcl_.point_step() + pcl_.fields()[3].offset()], &rgb_float, sizeof(float));
        memcpy(&pcl_.data()[i * pcl_.point_step() + pcl_.fields()[4].offset()], &intensity, sizeof(float));
        
        x_init+=0.1;
        y_init+=0.1;
        z_init+=0.1;
        
        r_color-=2;
        b_color++;

        uint32_t rgb = (static_cast<uint32_t>(r_color) << 16) |
                       (static_cast<uint32_t>(g_color) << 8) |
                       (static_cast<uint32_t>(b_color));

        memcpy(&rgb_float, &rgb, sizeof(rgb_float));
    }

    return pcl_;
}


int main()
{
    ChannelFactory::Instance()->Init(1, "lo");
    std::cout<< "Debug0"<<std::endl;

    ChannelSubscriber<unitree_go::msg::dds_::LowState_> lowstate_suber(TOPIC_LOWSTATE);
    lowstate_suber.InitChannel(LowStateHandler);
    std::cout<< "Debug1"<<std::endl;

    //ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> highstate_suber(TOPIC_HIGHSTATE);
    //highstate_suber.InitChannel(HighStateHandler);
    std::cout<< "Debug2"<<std::endl;

    ChannelPublisher<unitree_go::msg::dds_::LowCmd_> low_cmd_puber(TOPIC_LOWCMD);
    std::cout<< "Debug3"<<std::endl;
    low_cmd_puber.InitChannel();
    std::cout<< "Debug4"<<std::endl;

    ChannelPublisher<sensor_msgs::msg::dds_::PointCloud2_> pcl_publisher(TOPIC_PCL);
    pcl_publisher.InitChannel();
    std::cout<< "Debug5"<<std::endl;
    
    sensor_msgs::msg::dds_::PointCloud2_  pcl{};
    pcl = PointCloudCreator();

    while (true)
    {   
        unitree_go::msg::dds_::LowCmd_ low_cmd{};
        

        for (int i = 0; i < 20; i++)
        {
            low_cmd.motor_cmd()[i].q() = 0;
            low_cmd.motor_cmd()[i].kp() = 0;
            low_cmd.motor_cmd()[i].dq() = 0;
            low_cmd.motor_cmd()[i].kd() = 0;
            low_cmd.motor_cmd()[i].tau() = 0;
        }
        low_cmd_puber.Write(low_cmd);
        
        pcl_publisher.Write(pcl);


        usleep(2000);
    }

    return 0;
}
