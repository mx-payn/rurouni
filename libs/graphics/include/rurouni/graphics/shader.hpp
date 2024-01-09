#ifndef RR_LIB_GRAPHICS_SHADER_H
#define RR_LIB_GRAPHICS_SHADER_H

// rurouni
#include "rurouni/math/mat.hpp"
#include "rurouni/math/vec.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

// std
#include <vector>

namespace rr::graphics {

/** copy of glad.h */
enum class ShaderType : uint32_t {
    Vertex = 0x8B31,
    TessellationControl = 0x8E88,
    TessellationEvaluation = 0x8E87,
    Geometry = 0x8DD9,
    Fragment = 0x8B30,
    Compute = 0x91B9
};

struct ShaderSpecification {
    std::string Name = "unnamed";
    UUID Id = UUID();

    system::Path VertexFilepath = system::Path();
    system::Path FragmentFilepath = system::Path();
};

/** creates a shader program from vertex and fragment source. */
class Shader {
   public:
    /** reads and compiles the given shader sources and uploads them to GPU */
    Shader(const ShaderSpecification& spec);

    /** compiles the given shader sources and uploads them to GPU */
    Shader(const std::string& vertexSource, const std::string& fragmentSource);

    ~Shader();

    /** GPU call to bind / select a Shader for further processing.
     *  shaders have to be bound before data can be accessed on the GPU.
     */
    void bind() const;
    /** debug call to unbind / unselect a shader.
     *  effectively binds / selects shader with rendererid 0, which is the
     *  default shader.
     */
    void unbind() const;

    /** uploads a uniform int to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_Index").
     *  \param[in] value the value to be uploaded.
     */
    void set_int(const std::string& name, int value);
    /** uploads a uniform int[] to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_Samples").
     *  \param[in] value the value to be uploaded.
     */
    void set_int_array(const std::string& name, int* values, int32_t count);
    /** uploads a uniform float to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_TexIndex").
     *  \param[in] value the value to be uploaded.
     */
    void set_float(const std::string& name, float value);
    /** uploads a uniform float vec2 to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_2DPosition").
     *  \param[in] value the value to be uploaded.
     */
    void set_float_2(const std::string& name, const math::vec2& values);
    /** uploads a uniform float vec3 to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_3DPosition").
     *  \param[in] value the value to be uploaded.
     */
    void set_float_3(const std::string& name, const math::vec3& values);
    /** uploads a uniform float vec4 to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_Color").
     *  \param[in] value the value to be uploaded.
     */
    void set_float_4(const std::string& name, const math::vec4& values);
    /** uploads a uniform mat3 to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_ViewProjection").
     *  \param[in] value the value to be uploaded.
     */
    void set_mat_3(const std::string& name, const math::mat3& matrix);
    /** uploads a uniform mat4 to the shader location
     *
     *  \param[in] name the access identifier of uploaded uniform data inside
     *  the shader (e.g. "u_ViewProjection").
     *  \param[in] value the value to be uploaded.
     */
    void set_mat_4(const std::string& name, const math::mat4& matrix);

   private:
    /** reads the shader source code from file. */
    std::string get_shader_source(const std::filesystem::path& filepath);

    /** compiles the source code according for given shader type. */
    void compile_shader(const std::string& name,
                        const std::string& source,
                        ShaderType type,
                        uint32_t& program);

    /** links vertex and fragment shader into program. */
    void link_program(uint32_t& program);

   private:
    std::string m_Name;
    UUID m_Id;
    system::Path m_VertexPath;
    system::Path m_FragmentPath;
    std::vector<uint32_t> m_ShaderIDs;

    uint32_t m_RendererID;  //!< the unique identifier for data in GPU memory.
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRAPHICS_SHADER_H
