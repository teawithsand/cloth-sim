#include "object.h"
#include <iostream>

using namespace Util;

Object::Object() {}
Object::~Object()
{
    if (vao)
    {
        glDeleteVertexArrays(0, &vao);
        vao = 0;
    }
    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (ebo)
    {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
}

void Object::setupVAO()
{
    if (!vao || true)
    {
        assert(glGenVertexArrays);
        glGenVertexArrays(1, &vao);
    }
}

void Object::initVBO(const float *vertexData, int vertexSize, int vertexCount)
{
    setupVAO();
    if (!vbo)
    {
        glGenBuffers(1, &vbo);
    }

    this->indexCount = 0;
    this->vertexCount = vertexCount * vertexSize;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize * sizeof(float), vertexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, vertexSize, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Object::initVBOAndEBO(
    const float *vertexData, int vertexSize, int vertexCount,
    const unsigned int *indexData, int indexCount)
{
    setupVAO();
    if (!vbo || true)
    {
        glGenBuffers(1, &vbo);
    }
    if (!ebo || true)
    {
        glGenBuffers(1, &ebo);
    }

    this->indexCount = indexCount;
    this->vertexCount = vertexCount * vertexSize;

    glBindVertexArray(vao);

    // vbo setup
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize * sizeof(float), vertexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, vertexSize, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)0);

    // ebo setup
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indexData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Object::initVBOAndEBO2(
    const float *vertexData, int vertexSize1, int vertexSize2, int vertexCount,
    const unsigned int *indexData, int indexCount)
{
    setupVAO();
    if (!vbo || true)
    {
        glGenBuffers(1, &vbo);
    }
    if (!ebo || true)
    {
        glGenBuffers(1, &ebo);
    }
    const int vertexSize = vertexSize1 + vertexSize2;
    this->indexCount = indexCount;
    this->vertexCount = vertexCount * vertexSize;

    glBindVertexArray(vao);

    // vbo setup
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize * sizeof(float), vertexData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, vertexSize1, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, vertexSize2, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)vertexSize1);
    glEnableVertexAttribArray(1);
    // ebo setup
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void Object::updateVBO(const float *vertices, int vertexSize, int vertexCount)
{
    assert(this->vbo);
    assert(this->vao);

    this->vertexCount = vertexCount * vertexSize;
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize * sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, vertexSize, GL_FLOAT, GL_FALSE, vertexSize * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint Object::externalInit(int vertexCount, int indexCount)
{
    setupVAO();
    this->vertexCount = vertexCount;
    this->indexCount = indexCount;
    return vao;
}

GLuint Object::externalInitFinish(int vbo, int ebo)
{
    setupVAO();
    this->vbo = vbo;
    this->ebo = ebo;
    return vao;
}

void Object::draw()
{
    assert(vao);

    if (this->indexCount > 0)
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    else
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
}