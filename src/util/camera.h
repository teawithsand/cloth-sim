#include <glm/glm.hpp>
#include <memory>

#ifndef UTIL_CAMERA_H
#define UTIL_CAMERA_H

namespace Util
{
    class Camera
    {
    protected:
        Camera();

    public:
        virtual ~Camera(void);

        // whatever's needed for rendering goes here
        virtual glm::mat4 getView();
        virtual glm::mat4 getProjection();
    };

    class AzimuthCamera : public Camera
    {
    private:
        float radius = 100;
        float latitude = 0;
        float longitude = 0;
        float fov = 40;

        float center_x = 0;
        float center_y = 0;
        float center_z = 0;

        float screenHeight = 600;
        float screenWidth = 800;

    public:
        AzimuthCamera();

        inline void setRadius(float radius) { this->radius = radius; };
        inline void setLatitude(float latitude) { this->latitude = latitude; };
        inline void setLongitude(float longitude) { this->longitude = longitude; };
        inline void setFov(float fov) { this->fov = fov; };

        inline void setCenterX(float x) { this->center_x = x; };
        inline void setCenterY(float y) { this->center_y = y; };
        inline void setCenterZ(float z) { this->center_z = z; };

        inline void setScreenHeight(float h) { this->screenHeight = h; };
        inline void setScreenWidth(float w) { this->screenWidth = w; };

        inline float getRadius() { return this->radius; };
        inline float getLatitude() { return this->latitude; };
        inline float getLongitude() { return this->longitude; };
        inline float getFov() { return this->fov; };

        inline float getCenterX() { return this->center_x; };
        inline float getCenterY() { return this->center_y; };
        inline float getCenterZ() { return this->center_z; };


        glm::vec3 getViewer();

        glm::mat4 getView() override;
        glm::mat4 getProjection() override;
    };
}

#endif