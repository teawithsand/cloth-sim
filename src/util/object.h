#ifndef UTIL_OBJECT
#define UTIL_OBJECT

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Util
{
    class Object
    {
    private:
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLuint drawMode = GL_TRIANGLES;

        int indexCount = 0;
        int vertexCount = 0;

        void setupVAO();

    public:
        Object(void);
        ~Object(void);

        inline void setDrawMode(GLuint mode) {
            this->drawMode = mode;
        }

        void initVBO(const float *vertexData, int vertexSize, int vertexCount);
        void initVBOAndEBO(
            const float *vertexData, int vertexSize, int vertexCount,
            const unsigned int *indexData, int indexCount);

        void initVBOAndEBO2(
            const float *vertexData,
            int vertexSize1,
            int vertexSize2,
            int vertexCount,
            const unsigned int *indexData, int indexCount);

        GLuint externalInit(int vertexCount, int indexCount);
        GLuint externalInitFinish(int vbo, int ebo = 0);
        
        void updateVBO(const float *vertexData, int vertexSize, int vertexCount);

        void draw();
    };
}
#endif