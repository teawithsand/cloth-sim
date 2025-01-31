#ifndef UTIL_SHADER
#define UTIL_SHADER

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Util
{
    class Program
    {
    private:
        GLuint program = -1;

        std::string readFile(const char *filename);
        GLuint compileShader(GLenum shaderType,
                             std::string source,
                             const char *shaderName);

    public:
        Program(void);
        ~Program(void);
        GLuint getID()
        {
            return program;
        }

        void setupFiles(const char *vs, const char *fs);
        void setupRaw(const std::string &vs, const std::string &fs);

        GLint getLocation(const char *name);

        void use();

        /*
        inline void Program::setFloat(const char *name, float value)
        {
            auto loc = glGetUniformLocation(program, name);
            setFloat(loc, value);
        }
        inline void Program::setMat4(const char *name, glm::mat4 mat)
        {
            auto loc = glGetUniformLocation(program, name);
            setMat4(loc, mat);
        }
        */
        inline void setFloat(GLint loc, float value)
        {
            assert(loc >= 0);
            glUniform1f(loc, value);
        }
        inline void setMat4(GLint loc, glm::mat4 value)
        {
            assert(loc >= 0);
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        }
        inline void setVec3(GLint loc, glm::vec3 value)
        {
            assert(loc >= 0);
            glUniform3fv(loc, 1, glm::value_ptr(value));
        }
    };
}
#endif