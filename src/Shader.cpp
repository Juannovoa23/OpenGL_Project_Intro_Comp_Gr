// src/Shader.cpp
#include <string>
#include <glad/glad.h>
#include "Shader.h"

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShaderId = CreateShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShaderId = CreateShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (vertexShaderId != 0 && fragmentShaderId != 0) {
        id = CreateProgram(vertexShaderId, fragmentShaderId);
    }
    else {
        fprintf(stderr, "Could not create shader program\n");
        id = 0;
    }
}

Shader::~Shader() {
    if (id != 0)
        glDeleteProgram(id);
}

void Shader::Bind() const {
    glUseProgram(id);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

unsigned int Shader::CreateShader(unsigned int shaderType, const std::string& shaderSource) {
    unsigned int shaderId = glCreateShader(shaderType);
    SetShaderSource(shaderId, shaderSource);

    bool compiled = CompileShader(shaderId);
    if (compiled) {
        return shaderId;
    }
    else {
        glDeleteShader(shaderId);
        return 0;
    }
}

void Shader::SetShaderSource(unsigned int shaderId, const std::string& shaderSource) {
    const char* source = shaderSource.c_str();
    glShaderSource(shaderId, 1, &source, nullptr);
}

bool Shader::CompileShader(unsigned int shaderId) {
    glCompileShader(shaderId);

    int status = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
    bool compiled = status != 0;

    if (!compiled) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
        fprintf(stderr, "Shader compilation failed:\n%s\n", infoLog);
    }

    return compiled;
}

unsigned int Shader::CreateProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) {
    unsigned int programId = glCreateProgram();

    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    // *** IMPORTANTE ***
    // Vinculamos manualmente las localizaciones de atributos:
    //  aPos    -> location 0
    //  aNormal -> location 1
    // Esto sustituye al uso de "lay" en el shader.
    glBindAttribLocation(programId, 0, "aPos");
    glBindAttribLocation(programId, 1, "aNormal");

    bool linked = LinkProgram(programId);

    if (linked) {
        glDetachShader(programId, vertexShaderId);
        glDetachShader(programId, fragmentShaderId);
        return programId;
    }
    else {
        glDeleteProgram(programId);
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return 0;
    }
}

bool Shader::LinkProgram(unsigned int programId) {
    glLinkProgram(programId);

    int status = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    bool linked = status != 0;

    if (!linked) {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        fprintf(stderr, "Shader program linking failed:\n%s\n", infoLog);
    }

    return linked;
}
