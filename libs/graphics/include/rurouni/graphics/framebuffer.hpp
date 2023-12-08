#ifndef RR_LIB_GRPAHICS_FRAMEBUFFER_H
#define RR_LIB_GRPAHICS_FRAMEBUFFER_H

#include "rurouni/graphics/vertex_array.hpp"
#include "rurouni/math.hpp"

#include <memory>
#include <vector>

namespace rr::graphics {

enum class FramebufferTextureFormat {
    None = 0,

    // Color
    RGBA8,
    RED_INTEGER,

    // Depth
    DEPTH24STENCIL8,

    // Defaults
    Depth = DEPTH24STENCIL8
};

struct FramebufferTextureSpecification {
    FramebufferTextureSpecification() = default;
    FramebufferTextureSpecification(FramebufferTextureFormat format)
        : TextureFormat(format) {}

    FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
    // TODO: filtering/wrap
};

struct FramebufferAttachementSpecification {
    FramebufferAttachementSpecification(
        std::initializer_list<FramebufferTextureSpecification> attachements)
        : Attachements(attachements) {}

    std::vector<FramebufferTextureSpecification> Attachements;
};

struct FramebufferSpecification {
    FramebufferSpecification() = default;

    math::ivec2 size{0};
    FramebufferAttachementSpecification attachements{};
    uint32_t samples = 1;

    bool swapChainTarget = false;
};

class Framebuffer {
   public:
    Framebuffer(const FramebufferSpecification& spec);
    ~Framebuffer();

    void bind();
    void unbind();

    uint32_t get_color_attachment_renderer_id(uint32_t index = 0) const;
    uint32_t get_renderer_id() const { return m_RendererID; }
    uint32_t get_render_buffer_object() const { return m_RenderBufferObject; }
    const math::ivec2& get_size() const { return m_Specification.size; }

    void resize(const math::ivec2& size);
    int read_pixel(uint32_t attachementIndex, const math::ivec2& position);
    void clear_attachement(uint32_t attachementIndex, int value);

    void generate_screen_va();
    void draw_color_attachement(uint32_t index = 0);

   private:
    void cleanup();
    void invalidate();

   private:
    FramebufferSpecification m_Specification;
    uint32_t m_RendererID;
    uint32_t m_RenderBufferObject;

    std::unique_ptr<VertexArray> m_ScreenVA;

    std::vector<FramebufferTextureSpecification>
        m_ColorAttachementSpecifications;
    FramebufferTextureSpecification m_DepthAttachementSpecification{
        FramebufferTextureFormat::None};

    std::vector<uint32_t> m_ColorAttachements;
    uint32_t m_DepthAttachement = 0;
};

}  // namespace rr::graphics

#endif  // !RR_LIB_GRPAHICS_FRAMEBUFFER_H
