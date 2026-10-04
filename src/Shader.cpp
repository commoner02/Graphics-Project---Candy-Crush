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

out vec3 Normal;
out vec3 FragPos;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

static const char* DEFAULT_FRAG = R"(
#version 330 core
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec4 objectColor;
uniform bool isWireframe;

void main() {
    if (isWireframe) {
        // High-contrast clean outline for wireframe inspection
        FragColor = vec4(0.95, 0.95, 0.95, 1.0);
    } else {
        // Simple directional diffuse so solid mode showcases 3D depth
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.7));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 ambient = 0.35 * objectColor.rgb;
        vec3 diffuse = diff * 0.65 * objectColor.rgb;
        FragColor = vec4(ambient + diffuse, objectColor.a);
    }
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
