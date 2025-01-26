// MuJoCo header file
#include <mujoco/mujoco.h>
#include "cstdio"

#include <unitree/idl/ros2/PointCloud2_.hpp>
#include <time.h>


class PointCloudGenerator
{
    private:
    u_char* color_buffer;
    float* depth_buffer;

    float extent; // Depth scale (m)
    float z_near; // Near clipping plane depth
    float z_far;  // Far clipping plane depth
    float fx;      // Focal length
    float fy;
    int cx, cy;    // Principal points
    float fovy;

    mjrRect viewport;

    void linearize_depth(float * depth);

    public:
    PointCloudGenerator(){};
    ~PointCloudGenerator(){};

    void set_camera_intrinsics(const mjsCamera * camera);

    void get_RGBD_buffer(const mjModel* model, const mjrContext* context);

    inline void release_buffer()
    {
      free(color_buffer);
      free(depth_buffer);
    }

    /// @brief Generate colorful pointcloud
    /// @return colorful pointcloud
    sensor_msgs::msg::dds_::PointCloud2_ generate_color_pointcloud(void);

    mjrRect get_viewport() const { return this->viewport;};

};
