#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

static const char* DEFAULT_VERT = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int shadingMode;
uniform vec3 viewPos;
uniform vec4 objectColor;

out vec3 Normal;
out vec3 FragPos;
out vec3 vGouraudColor;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);

    if (shadingMode == 1) {
        vec3 lightPos = vec3(7.0, 14.0, 7.0);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        vec3 base = objectColor.rgb;
        vec3 ambient = 0.25 * base;
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * 0.65 * base;
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
        vec3 specular = spec * 0.45 * vec3(1.0);

        vGouraudColor = ambient + diffuse + specular;
    } else {
        vGouraudColor = objectColor.rgb;
    }
}
)";

static const char* DEFAULT_FRAG = R"(
#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec3 vGouraudColor;

out vec4 FragColor;

uniform vec4 objectColor;
uniform bool isWireframe;
uniform bool isTray;
uniform int shadingMode;
uniform vec3 viewPos;

void main() {
    if (isWireframe) {
        FragColor = isTray ? vec4(0.38, 0.42, 0.52, 1.0) : vec4(objectColor.rgb, objectColor.a);
        return;
    }
    if (shadingMode == 3) {
        vec3 n = normalize(Normal);
        FragColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }
    if (shadingMode == 1 && !isTray) {
        FragColor = vec4(vGouraudColor, objectColor.a);
        return;
    }
    vec3 lightPos = vec3(7.0, 14.0, 7.0);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 baseColor = isTray ? vec3(0.32, 0.36, 0.44) : objectColor.rgb;

    if (shadingMode == 0) {
        vec3 ambient = 0.35 * baseColor;
        vec3 diffuse = diff * 0.65 * baseColor;
        FragColor = vec4(ambient + diffuse, objectColor.a);
        return;
    }

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec3 ambient = 0.22 * baseColor;
    vec3 diffuse = diff * 0.68 * baseColor;
    float spec = pow(max(dot(norm, halfwayDir), 0.0), isTray ? 16.0 : 64.0);
    vec3 specular = spec * (isTray ? 0.20 : 0.65) * vec3(1.0);
    FragColor = vec4(ambient + diffuse + specular, objectColor.a);
}
)";

Shader::~Shader() {
    destroy();
}

Shader::Shader(Shader&& other) noexcept : programId(other.programId) {
    other.programId = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        destroy();
        programId = other.programId;
        other.programId = 0;
    }
    return *this;
}

bool Shader::init(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vCode, fCode;
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);

    if (vFile.is_open() && fFile.is_open()) {
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        vCode = vStream.str();
        fCode = fStream.str();
        return initFromSource(vCode.c_str(), fCode.c_str());
    } else {
        std::cout << "[Shader] Using embedded default shaders (could not find external file)." << std::endl;
        return initFromSource(DEFAULT_VERT, DEFAULT_FRAG);
    }
}

bool Shader::initFromSource(const char* vertexCode, const char* fragmentCode) {
    destroy();

    GLuint vShader = compileShader(GL_VERTEX_SHADER, vertexCode);
    if (!vShader) return false;

    GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);
    if (!fShader) {
        glDeleteShader(vShader);
        return false;
    }

    programId = glCreateProgram();
    glAttachShader(programId, vShader);
    glAttachShader(programId, fShader);
    glLinkProgram(programId);

    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, nullptr, infoLog);
        std::cerr << "[Shader Link Error]: " << infoLog << std::endl;
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        destroy();
        return false;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return true;
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "[Shader Compile Error (" << (type == GL_VERTEX_SHADER ? "VERT" : "FRAG") << ")]: "
                  << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Shader::use() const {
    if (programId != 0) {
        glUseProgram(programId);
    }
}

void Shader::destroy() {
    if (programId != 0) {
        glDeleteProgram(programId);
        programId = 0;
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec4(const std::string& name, const glm::vec4& vec) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string& name, float val) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1f(loc, val);
}

void Shader::setInt(const std::string& name, int val) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1i(loc, val);
}

void Shader::setBool(const std::string& name, bool val) const {
    GLint loc = glGetUniformLocation(programId, name.c_str());
    if (loc != -1) glUniform1i(loc, val ? 1 : 0);
}
