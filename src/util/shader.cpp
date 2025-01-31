#include "shader.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace Util;

Program::Program() {};
Program::~Program()
{
    if (program != -1)
    {
        glDeleteProgram(program);
        program = -1;
    }
};

std::string Program::readFile(const char *filename)
{
    std::string shaderCode;
    std::ifstream file(filename, std::ios::in);

    if (!file.good())
    {
        std::cout << "Can't read file " << filename << std::endl;
        std::terminate();
    }

    file.seekg(0, std::ios::end);
    shaderCode.resize((unsigned int)file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&shaderCode[0], shaderCode.size());
    file.close();
    return shaderCode;
}

GLuint Program::compileShader(GLenum shaderType, std::string source, const char *shaderName)
{
    int compile_result = 0;

    GLuint shader = glCreateShader(shaderType);
    const char *shader_code_ptr = source.c_str();
    const int shader_code_size = source.size();

    glShaderSource(shader, 1, &shader_code_ptr, &shader_code_size);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

    if (compile_result == GL_FALSE)
    {
        int info_log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
        std::vector<char> shader_log(info_log_length);
        glGetShaderInfoLog(shader, info_log_length, NULL, &shader_log[0]);
        std::cout << "ERROR compiling shader: " << shaderName << std::endl
                  << &shader_log[0] << std::endl;
        return 0;
    }

    return shader;
}

void Program::setupFiles(const char *vs, const char *fs)
{
    auto vsData = readFile(vs);
    auto fsData = readFile(fs);
    Program::setupRaw(
        vsData,
        fsData);
}

void Program::setupRaw(
    const std::string &vs,
    const std::string &fs)
{
    GLuint vsCompiled = compileShader(
        GL_VERTEX_SHADER,
        vs,
        "vertex shader");

    GLuint fsCompiled = compileShader(
        GL_FRAGMENT_SHADER,
        fs,
        "fragment shader");

    int link_result = 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vsCompiled);
    glAttachShader(program, fsCompiled);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_result);
    if (link_result == GL_FALSE)
    {
        int info_log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
        std::vector<char> programLog(info_log_length);
        glGetProgramInfoLog(program, info_log_length, NULL, &programLog[0]);
        std::cout << "Shader Loader : LINK ERROR" << std::endl
                  << &programLog[0] << std::endl;
    }

    glDetachShader(program, fsCompiled);
    glDetachShader(program, vsCompiled);
    glDeleteShader(fsCompiled);
    glDeleteShader(vsCompiled);

    this->program = program;
}

void Program::use()
{
    glUseProgram(program);
}

GLint Program::getLocation(const char *name)
{
    return glGetUniformLocation(program, name);
}

