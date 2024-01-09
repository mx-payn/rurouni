// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/shader.hpp"
#include "rurouni/math.hpp"
#include "rurouni/system/filesystem.hpp"

// external
#include <glad/gl.h>

namespace rr::graphics {

// TODO default shaders

Shader::Shader(const ShaderSpecification& spec)
    : m_Name(spec.Name), m_Id(spec.Id) {
    uint32_t program = glCreateProgram();

    compile_shader(spec.VertexFilepath.filename(),
                   get_shader_source(spec.VertexFilepath), ShaderType::Vertex,
                   program);
    compile_shader(spec.FragmentFilepath.filename(),
                   get_shader_source(spec.FragmentFilepath),
                   ShaderType::Fragment, program);

    link_program(program);

    m_RendererID = program;
}

Shader::Shader(const std::string& vertexSource,
               const std::string& fragmentSource) {
    uint32_t program = glCreateProgram();

    compile_shader("default.vs", vertexSource, ShaderType::Vertex, program);

    compile_shader("default.fs", fragmentSource, ShaderType::Fragment, program);

    link_program(program);

    m_RendererID = program;
}

Shader::~Shader() {
    glDeleteProgram(m_RendererID);
}

std::string Shader::get_shader_source(const system::Path& filepath) {
    // read shader source
    std::string shaderSource;

    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (in) {
        in.seekg(0, std::ios::end);
        shaderSource.resize((size_t)in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&shaderSource[0], (int32_t)shaderSource.size());
        in.close();
    } else {
        error("Could not open file: {}", filepath.c_str());
    }

    return shaderSource;
}

void Shader::compile_shader(const std::string& name,
                            const std::string& sourceCode,
                            ShaderType type,
                            uint32_t& program) {
    GLuint shader = glCreateShader(static_cast<uint32_t>(type));

    const GLchar* source = sourceCode.c_str();
    glShaderSource(shader, 1, &source, 0);

    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog((size_t)maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        // We don't need the shader anymore.
        glDeleteShader(shader);

        error("{}", infoLog.data());
        require(false, "shader compilation failure!");
    }

    glAttachShader(program, shader);
    m_ShaderIDs.push_back(shader);
}

void Shader::link_program(uint32_t& program) {
    // Link our m_RendererID
    glLinkProgram(program);

    // Note the different functions here: glGetM_RendererID* instead of
    // glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog((size_t)maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        // We don't need the program anymore.
        glDeleteProgram(program);
        // Don't leak shaders either.
        //
        for (auto id : m_ShaderIDs)
            glDeleteShader(id);

        error("{}", infoLog.data());
        require(false, "Shader link failure!");
        return;
    }

    // Always detach shaders after a successful link.
    //
    for (auto id : m_ShaderIDs)
        glDetachShader(program, id);
}

void Shader::bind() const {
    glUseProgram(m_RendererID);
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::set_int(const std::string& name, int value) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform1i(location, value);
}

void Shader::set_int_array(const std::string& name,
                           int* values,
                           int32_t count) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform1iv(location, count, values);
}

void Shader::set_float(const std::string& name, float value) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform1f(location, value);
}

void Shader::set_float_2(const std::string& name, const math::vec2& values) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform2f(location, values.x, values.y);
}

void Shader::set_float_3(const std::string& name, const math::vec3& values) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform3f(location, values.x, values.y, values.z);
}

void Shader::set_float_4(const std::string& name, const math::vec4& values) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniform4f(location, values.x, values.y, values.z, values.w);
}

void Shader::set_mat_3(const std::string& name, const math::mat3& matrix) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniformMatrix3fv(location, 1, GL_FALSE, math::value_ptr(matrix));
}

void Shader::set_mat_4(const std::string& name, const math::mat4& matrix) {
    GLint location = glGetUniformLocation(m_RendererID, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, math::value_ptr(matrix));
}

}  // namespace rr::graphics
