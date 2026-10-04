#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    GLuint programId = 0;

    Shader() = default;
    ~Shader();

    // Prevent copying
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Move semantics
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    // Load from disk with fallback to embedded defaults if file is missing
    bool init(const std::string& vertexPath, const std::string& fragmentPath);
    bool initFromSource(const char* vertexCode, const char* fragmentCode);

    void use() const;
    void destroy();

    // Uniform setters
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec4(const std::string& name, const glm::vec4& vec) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setFloat(const std::string& name, float val) const;
    void setInt(const std::string& name, int val) const;
    void setBool(const std::string& name, bool val) const;

private:
    static GLuint compileShader(GLenum type, const char* source);
};
