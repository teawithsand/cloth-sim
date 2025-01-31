
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <random>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTX_color_space

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>

#include "util/object.h"
#include "util/shader.h"
#include "util/camera.h"
#include "util/sphere.h"
#include "sim/cloth.h"
#include "util/render.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static const char *vertexShaderSource = R"=====(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 transform;
uniform mat4 projection;
uniform mat4 view;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} vs_out;

void main()
{
    vs_out.FragPos = aPos;
    vs_out.Normal = vec3(0, 1, 0);
	gl_Position = projection * view * transform * vec4(aPos.xyz, 1.0);
}
)=====";

static const char *fragmentShaderSource = R"=====(
#version 330 core

uniform vec3 color;
uniform vec3 viewPos;
uniform vec3 lightPos;
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

void main()
{
    /*
    // ambient
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 normal = normalize(fs_in.Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(true)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    */

    FragColor = vec4(color.x, color.y, color.z, 1.0);
}
)=====";

const int CLOTH_BALLS_WIDTH = 20;
const int CLOTH_BALLS_HEIGHT = 20;
float clothEdge = 10.0f / ((CLOTH_BALLS_WIDTH + CLOTH_BALLS_WIDTH) / 2.0f);
float dt = 1.0f / 10.0f;
int stepsPerFrame = 1;

glm::vec3 colliderPosition = glm::vec3(0, 0, 0);

Util::AzimuthCamera camera;
Sim::Cloth clothSim;

Sim::ClothCollider c1;

void imgui_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags imguiWindowFlags = 0;
    // imguiWindowFlags |= ImGuiWindowFlags_NoCollapse;
    // imguiWindowFlags |= ImGuiWindowFlags_NoResize;
    // imguiWindowFlags |= ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowBgAlpha(0.8f);

    ImGui::Begin("Control", nullptr, imguiWindowFlags);

    if (ImGui::CollapsingHeader("Camera"))
    {
        float latitude = camera.getLatitude();
        ImGui::SliderFloat("Latitude", &latitude, -90, 90);
        camera.setLatitude(latitude);

        float longitude = camera.getLongitude();
        ImGui::SliderFloat("Longitude", &longitude, -180, 180);
        camera.setLongitude(longitude);

        float radius = camera.getRadius();
        ImGui::SliderFloat("Radius", &radius, 0, 200);
        camera.setRadius(radius);

        float center[3] = {camera.getCenterX(), camera.getCenterY(), camera.getCenterZ()};
        ImGui::SliderFloat3("Center", center, -100, 100);
        camera.setCenterX(center[0]);
        camera.setCenterY(center[1]);
        camera.setCenterZ(center[2]);
    }

    if (ImGui::CollapsingHeader("Springs"))
    {
        ImGui::SliderFloat("Spring damping", &clothSim.damping, 0.0f, 1.0f);
        ImGui::SliderFloat("Spring hardness", &clothSim.springHardness, 0.0f, 60.0f);
    }

    if (ImGui::CollapsingHeader("Sim"))
    {
        auto gravity = clothSim.getGravity();

        ImGui::SliderFloat3("Gravity", glm::value_ptr(gravity), -1, 1);

        if (ImGui::Button("Zero gravity"))
        {
            gravity = glm::vec3(0);
        }
        clothSim.setGravity(gravity);

        // any bigger value causes all the triangles to fly away due to float arithmetic resulting
        // in some errors.
        // in fact even 1/5 is too much

        ImGui::SliderFloat("Sim Step", &dt, 1.0f / 144.0f, 1.0f / 5.0f);
        if (ImGui::Button("Reset sim step"))
        {
            dt = 1.0f / 10.0f;
        }

        ImGui::SliderInt("Steps per frame", &stepsPerFrame, 1, 10);
    }

    if (ImGui::CollapsingHeader("Collider"))
    {
        auto gravity = clothSim.getGravity();

        ImGui::SliderFloat("Collision correction coef", &clothSim.collisionCorrectionTerm, 0.1f, 1.0f);
        ImGui::SliderFloat3("Collider center", glm::value_ptr(c1.x), -30, 30);
        ImGui::SliderFloat("Collider radius", &c1.radius, 1, 30);
        ImGui::SliderFloat("Collider radius epsilon", &c1.radiusEpsilon, 0, 1);

        if (ImGui::Button("Reset collider"))
        {
            c1.radius = 5;
            c1.radiusEpsilon = 0.5;
            c1.x = glm::vec3(5.0f, -20, 5.0f);
        }
    }

    if (ImGui::CollapsingHeader("Cloth points"))
    {
        auto gravity = clothSim.getGravity();

        ImGui::Checkbox("Lock (0,0) point", &clothSim.getPointPtr(0, 0)->frozen);
        ImGui::Checkbox("Lock (W,0) point", &clothSim.getPointPtr(clothSim.getWidth() - 1, 0)->frozen);
        ImGui::Checkbox("Lock (0,H) point", &clothSim.getPointPtr(0, clothSim.getHeight() - 1)->frozen);
        ImGui::Checkbox("Lock (W,H) point", &clothSim.getPointPtr(clothSim.getWidth() - 1, clothSim.getHeight() - 1)->frozen);

        // This (float *) hack abuses the fact that vec3 is also pointer to 3 floats
        ImGui::SliderFloat3("(0,0)", glm::value_ptr(clothSim.getPointPtr(0, 0)->x), -10, 10);
        ImGui::SliderFloat3("(W,0)", glm::value_ptr(clothSim.getPointPtr(clothSim.getWidth() - 1, 0)->x), -20, 20);
        ImGui::SliderFloat3("(0,H)", glm::value_ptr(clothSim.getPointPtr(0, clothSim.getHeight() - 1)->x), -20, 20);
        ImGui::SliderFloat3("(W,H)", glm::value_ptr(clothSim.getPointPtr(clothSim.getWidth() - 1, clothSim.getHeight() - 1)->x), -20, 20);

        if (ImGui::Button("Reset corners"))
        {
            for (int x : {0, clothSim.getWidth() - 1})
            {
                for (int y : {0, clothSim.getHeight() - 1})
                {
                    clothSim.getPointPtr(x, y)->frozen = true;
                    // hack: change plane on which cloth is being projected
                    // that's why we set z to value of variable y
                    clothSim.getPointPtr(x, y)->x = glm::vec3(x * clothEdge, 0, y * clothEdge);
                    clothSim.getPointPtr(x, y)->xPrev = glm::vec3(0);
                    clothSim.getPointPtr(x, y)->v = glm::vec3(0);
                }
            }
        }

        if (ImGui::Button("Unlock corners"))
        {
            for (int x : {0, clothSim.getWidth() - 1})
            {
                for (int y : {0, clothSim.getHeight() - 1})
                {
                    clothSim.getPointPtr(x, y)->frozen = false;
                }
            }
        }
    }

    ImGui::End();
}

int ballCoords(int x, int y)
{
    return x * CLOTH_BALLS_HEIGHT + y;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL app", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);

    const char *glslVersion = "#version 460 core";
    ImGui_ImplOpenGL3_Init(glslVersion);

    std::cerr << "Version: " << glGetString(GL_VERSION) << "\n";
    ImGui::StyleColorsDark();

    Util::Program shader;

    clothSim.init(CLOTH_BALLS_WIDTH, CLOTH_BALLS_HEIGHT, clothEdge);
    clothSim.setGravity(glm::vec3(0.0f, -1, 0));

    std::vector<unsigned int> indices;
    std::vector<float> vertices;

    Util::Object drawObj;
    for (std::size_t x = 0; x < CLOTH_BALLS_WIDTH; x++)
    {
        for (std::size_t y = 0; y < CLOTH_BALLS_HEIGHT; y++)
        {
            auto ball = clothSim.getPoint(x, y);
            vertices.push_back(ball.x.x);
            vertices.push_back(ball.x.y);
            vertices.push_back(ball.x.z);

            // Normal
            vertices.push_back(0);
            vertices.push_back(1);
            vertices.push_back(0);

            if (x < CLOTH_BALLS_WIDTH - 1 && y < CLOTH_BALLS_HEIGHT - 1)
            {
                // 1st triangle
                indices.push_back(ballCoords(x, y));
                indices.push_back(ballCoords(x + 1, y));
                indices.push_back(ballCoords(x + 1, y + 1));
                // 2nd triangle
                indices.push_back(ballCoords(x + 1, y + 1));
                indices.push_back(ballCoords(x, y + 1));
                indices.push_back(ballCoords(x, y));
            }
        }
    }
    drawObj.initVBOAndEBO2(
        vertices.data(),
        3,
        3,
        CLOTH_BALLS_WIDTH * CLOTH_BALLS_HEIGHT,
        indices.data(),
        indices.size());

    Util::Sphere sphere;
    Util::Object sphereObj;

    sphereObj.initVBOAndEBO(
        sphere.getVertices(),
        3,
        sphere.getVertexCount(),
        sphere.getIndices(),
        sphere.getIndexCount());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // glEnable(GL_CULL_FACE);

    // glDisable(GL_CULL_FACE);
    // glDepthFunc(GL_LESS);
    // glFrontFace(GL_CCW);

    shader.setupRaw(
        vertexShaderSource,
        fragmentShaderSource);

    shader.use();
    const auto projectionLoc = shader.getLocation("projection");
    const auto viewLoc = shader.getLocation("view");
    const auto transformLoc = shader.getLocation("transform");
    const auto colorLoc = shader.getLocation("color");
    const auto viewPosLoc = shader.getLocation("viewPos");
    const auto lightPosLoc = shader.getLocation("lightPos");

    // clothSim.setGravity(glm::vec3(0.0f));
    clothSim.getPointPtr(0, 0)->frozen = true;
    clothSim.getPointPtr(clothSim.getWidth() - 1, 0)->frozen = true;
    clothSim.getPointPtr(clothSim.getWidth() - 1, clothSim.getHeight() - 1)->frozen = true;
    clothSim.getPointPtr(0, clothSim.getHeight() - 1)->frozen = true;

    c1.radius = 5;
    c1.radiusEpsilon = 0.5;
    c1.x = glm::vec3(5.0f, -20, 5.0f);
    clothSim.colliders.push_back(&c1);

    camera.setCenterX(c1.x.x);
    camera.setCenterY(c1.x.y);
    camera.setCenterZ(c1.x.z);

    while (!glfwWindowShouldClose(window))
    {
        process_input(window);
        imgui_frame();

        for (int x = 0; x < clothSim.getWidth(); x++)
        {
            for (int y = 0; y < clothSim.getHeight(); y++)
            {
                auto offset = ballCoords(x, y) * 3;
                auto pos = clothSim.getPoint(x, y).x;
                vertices[offset + 0] = pos.x;
                vertices[offset + 1] = pos.y;
                vertices[offset + 2] = pos.z;
            }
        }
        drawObj.updateVBO(vertices.data(), 3, clothSim.getWidth() * clothSim.getHeight());

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 transform(1);

        shader.use();
        shader.setMat4(projectionLoc, camera.getProjection());
        shader.setMat4(viewLoc, camera.getView());
        shader.setMat4(transformLoc, transform);
        shader.setVec3(colorLoc, glm::vec3(0.5, 0.5, 0.5));

        // shader.setVec3(viewPosLoc, camera.getViewer());
        // shader.setVec3(lightPosLoc, glm::vec3(0, 0, 0));

        drawObj.draw();

        for (const auto &collider : clothSim.colliders)
        {
            transform = glm::mat4(1);

            transform = glm::translate(transform, collider->x);
            transform = glm::scale(transform, glm::vec3(collider->radius));
            shader.setMat4(transformLoc, transform);
            shader.setVec3(colorLoc, glm::vec3(1, 0, 0));
            sphereObj.draw();
        }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        for (int i = 0; i < stepsPerFrame; i++)
        {
            clothSim.step(dt);
        }
    }

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.setScreenWidth(width);
    camera.setScreenHeight(height);
}
