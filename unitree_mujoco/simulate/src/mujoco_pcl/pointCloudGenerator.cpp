#include "pointCloudGenerator.hpp"

 void PointCloudGenerator::linearize_depth(float * depth)
{
    // Allocation pour le tableau temporaire
    float *lin_depth = new float[viewport.width * viewport.height];

    // Copier les données du buffer d'entrée
    memcpy(lin_depth, depth, viewport.width * viewport.height * sizeof(float));

    for (int i = 0; i < viewport.width*viewport.height; i++)
    {
        depth[i] = z_near * z_far * extent / (z_far - lin_depth[i] * (z_far - z_near));
    }

    delete[] lin_depth;
    
}

void PointCloudGenerator::set_camera_intrinsics(const mjsCamera * camera)
{
    // Viewport dimensions
    viewport.width = camera->resolution[0];
    viewport.height = camera->resolution[1];

    // Vertical FOV
    fovy = ((float)(camera->fovy) / 180 ) * M_PI / 2;

    // Focal length, fx = fy
    fx = camera->focal_pixel[0];
    fy = camera->focal_pixel[0];

    // Principal points
    cx = viewport.width / 2;
    cy = viewport.height / 2;

}

void PointCloudGenerator::get_RGBD_buffer(const mjModel* model, const mjrContext* context)
{
    color_buffer = (u_char*) malloc(viewport.height*viewport.width * 3);
    depth_buffer = (float*) malloc(viewport.height*viewport.width * sizeof(float));
    mjr_readPixels(color_buffer, depth_buffer, viewport, context);

    extent = model->stat.extent;
    z_near = model->vis.map.znear;
    z_far = model->vis.map.zfar;
    
    linearize_depth(depth_buffer);

}

sensor_msgs::msg::dds_::PointCloud2_  PointCloudGenerator::generate_color_pointcloud(void)
{
    sensor_msgs::msg::dds_::PointCloud2_ pcl_{};
    
    time_t sec;
    time(&sec);
    pcl_.header().stamp().sec() = sec;
    pcl_.header().frame_id() = "map";

    pcl_.height() = 1; // Unorganised pointcloud
    pcl_.width() = viewport.width * viewport.height;
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

    pcl_.fields()[4].name() = "a";
    pcl_.fields()[4].offset() = 16;
    pcl_.fields()[4].datatype() =  sensor_msgs::msg::dds_::PointField_Constants::FLOAT32_;
    pcl_.fields()[4].count() = 1;
    
    pcl_.point_step() = 20; // Number of bytes used by on point
    pcl_.is_bigendian() = false;
    pcl_.row_step() = pcl_.point_step() * pcl_.width();
    pcl_.data().resize(pcl_.row_step() * pcl_.height());

    // Using this as a log
    // pcl_.header().stamp().sec() = z_near;
    // pcl_.header().stamp().nanosec() = context->currentBuffer;
    // //pcl_.header().frame_id() = std::to_string(extent);

    float intensity = 0.5;

    for (int y_index = 0 ; y_index < viewport.height ; y_index++)
    {
        for (int x_index = 0 ; x_index < viewport.width ; x_index++)
        {
            int index = y_index * viewport.width + x_index; 
            float depth_point = depth_buffer[index];

            if (depth_point < z_far)
            {
                // point computation block
                float x = float(x_index - cx) * depth_point / fx;
                float y = float(y_index - cy) * depth_point / fy;
                float z = depth_point;

                // color computation block
                uint8_t r_color = color_buffer[3 * index];
                uint8_t g_color = color_buffer[3 * index + 1];
                uint8_t b_color = color_buffer[3 * index + 2];

                uint32_t rgb =  (static_cast<uint32_t>(r_color) << 16) |
                                (static_cast<uint32_t>(g_color) << 8) |
                                (static_cast<uint32_t>(b_color));

                // memcpy block
                memcpy(&pcl_.data()[index * pcl_.point_step() + pcl_.fields()[0].offset()], &x, sizeof(float));
                memcpy(&pcl_.data()[index * pcl_.point_step() + pcl_.fields()[1].offset()], &y, sizeof(float));
                memcpy(&pcl_.data()[index * pcl_.point_step() + pcl_.fields()[2].offset()], &z, sizeof(float));
                memcpy(&pcl_.data()[index * pcl_.point_step() + pcl_.fields()[3].offset()], &rgb, sizeof(float));
                memcpy(&pcl_.data()[index * pcl_.point_step() + pcl_.fields()[4].offset()], &intensity, sizeof(float));
            }
        }
    }

    return pcl_;
}

