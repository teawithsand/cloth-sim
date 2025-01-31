#include "camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTX_color_space

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace Util;

Camera::Camera() {}
Camera::~Camera() {}
glm::mat4 Camera::getProjection()
{
    return glm::mat4(1.0f);
}
glm::mat4 Camera::getView()
{
    return glm::mat4(1.0f);
}

AzimuthCamera::AzimuthCamera()
{
}

glm::mat4 AzimuthCamera::getProjection()
{
    return glm::perspective(
        (float)(((float)fov) / 180.0f * M_PI),
        screenWidth / screenHeight,
        0.1f,
        100000.0f);
}
glm::mat4 AzimuthCamera::getView()
{

    float camX = radius * cos(glm::radians(this->latitude)) * cos(glm::radians(this->longitude));
    float camZ = radius * cos(glm::radians(this->latitude)) * sin(glm::radians(this->longitude));
    float camY = radius * sin(glm::radians(this->latitude));

    auto center = glm::vec3(this->center_x, this->center_y, this->center_z);
    auto cam_pos = center + glm::vec3(camX, camY, camZ);

    glm::mat4 res = glm::lookAt(
        cam_pos,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f));

    return res;
}

glm::vec3 AzimuthCamera::getViewer()
{

    float camX = radius * cos(glm::radians(this->latitude)) * cos(glm::radians(this->longitude));
    float camZ = radius * cos(glm::radians(this->latitude)) * sin(glm::radians(this->longitude));
    float camY = radius * sin(glm::radians(this->latitude));

    return glm::vec3(camX, camY, camZ);
}