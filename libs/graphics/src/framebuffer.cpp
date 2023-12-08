// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/dev/logger.hpp"
#include "rurouni/graphics/framebuffer.hpp"
#include "rurouni/math.hpp"

// external
#include <glad/gl.h>

namespace rr::graphics {

static bool is_depth_format(FramebufferTextureFormat format) {
    switch (format) {
        case FramebufferTextureFormat::DEPTH24STENCIL8:
            return true;
        default:
            return false;
    }
}

static GLenum rurouni_fb_texture_format_to_gl(FramebufferTextureFormat format) {
    switch (format) {
        case FramebufferTextureFormat::RGBA8:
            return GL_RGBA8;

        case FramebufferTextureFormat::RED_INTEGER:
            return GL_RED_INTEGER;

        default:
            dev::LOG->require(false, "Unknown FramebufferTextureFormat!");
            return 0;
    }
}

static GLenum texture_target(bool multisampled) {
    return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static void bind_texture(bool multisampled, uint32_t id) {
    glBindTexture(texture_target(multisampled), id);
}

static void create_textures(bool multisampled,
                            uint32_t* outID,
                            uint32_t count) {
    glCreateTextures(texture_target(multisampled), count, outID);
}

static void attach_color_texture(uint32_t id,
                                 int samples,
                                 GLenum internalFormat,
                                 GLenum format,
                                 const math::ivec2& size,
                                 int index) {
    bool multisampled = samples > 1;
    if (multisampled) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,
                                internalFormat, size.x, size.y, GL_FALSE);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0,
                     format, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                           texture_target(multisampled), id, 0);
}

static void attach_depth_texture(uint32_t id,
                                 int samples,
                                 GLenum format,
                                 GLenum attachementType,
                                 const math::ivec2& size) {
    bool multisampled = samples > 1;
    if (multisampled) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format,
                                size.x, size.y, GL_FALSE);
    } else {
        glTexStorage2D(GL_TEXTURE_2D, 1, format, size.x, size.y);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachementType,
                           texture_target(multisampled), id, 0);
}

const uint32_t quadVertexCount = 24;
float quadVertices[quadVertexCount] = {
    // vertex attributes for a quad that fills the
    // specified min and max positions
    // in Normalized Device Coordinates.
    // positions   // texCoords
    // clang-format off
      -1.0f,  1.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f,
       1.0f, -1.0f, 1.0f, 0.0f,

      -1.0f,  1.0f, 0.0f, 1.0f,
       1.0f, -1.0f, 1.0f, 0.0f,
       1.0f,  1.0f, 1.0f, 1.0f
    // clang-format on
};

BufferLayout quadLayout = {{ShaderDataType::Float2, "a_Position"},
                           {ShaderDataType::Float2, "a_TexCoords"}};

Framebuffer::Framebuffer(const FramebufferSpecification& spec)
    : m_Specification(spec) {
    for (auto spec : m_Specification.attachements.Attachements) {
        if (!is_depth_format(spec.TextureFormat))
            m_ColorAttachementSpecifications.emplace_back(spec.TextureFormat);
        else
            m_DepthAttachementSpecification = spec.TextureFormat;
    }

    m_ScreenVA = std::make_unique<VertexArray>(quadVertices, quadVertexCount,
                                               quadLayout);

    invalidate();
}

Framebuffer::~Framebuffer() {
    cleanup();
}

void Framebuffer::cleanup() {
    glDeleteFramebuffers(1, &m_RendererID);
    glDeleteTextures(m_ColorAttachements.size(), m_ColorAttachements.data());
    glDeleteTextures(1, &m_DepthAttachement);

    m_ColorAttachements.clear();
    m_DepthAttachement = 0;
}

void Framebuffer::invalidate() {
    if (m_RendererID)
        cleanup();

    auto size = m_Specification.size;
    glGenFramebuffers(1, &m_RendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

    bool multisample = m_Specification.samples > 1;

    if (m_ColorAttachementSpecifications.size()) {
        m_ColorAttachements.resize(m_ColorAttachementSpecifications.size());
        create_textures(multisample, m_ColorAttachements.data(),
                        m_ColorAttachements.size());

        // color attachement
        for (int i = 0; i < m_ColorAttachements.size(); i++) {
            bind_texture(multisample, m_ColorAttachements[i]);
            switch (m_ColorAttachementSpecifications[i].TextureFormat) {
                case FramebufferTextureFormat::RGBA8:
                    attach_color_texture(m_ColorAttachements[i],
                                         m_Specification.samples, GL_RGBA8,
                                         GL_RGBA, m_Specification.size, i);
                    break;
                case FramebufferTextureFormat::RED_INTEGER:
                    attach_color_texture(
                        m_ColorAttachements[i], m_Specification.samples,
                        GL_R32I, GL_RED_INTEGER, m_Specification.size, i);
                    break;
                default:
                    break;
            }
        }
    }

    if (m_DepthAttachementSpecification.TextureFormat !=
        FramebufferTextureFormat::None) {
        create_textures(multisample, &m_DepthAttachement, 1);
        bind_texture(multisample, m_DepthAttachement);
        switch (m_DepthAttachementSpecification.TextureFormat) {
            case FramebufferTextureFormat::DEPTH24STENCIL8:
                attach_depth_texture(
                    m_DepthAttachement, m_Specification.samples,
                    GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT,
                    m_Specification.size);
                break;

            default:
                break;
        }
    }

    if (m_ColorAttachements.size() > 1) {
        dev::LOG->require(m_ColorAttachements.size() <= 4,
                          "incompatible color attachement count");
        GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                             GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(m_ColorAttachements.size(), buffers);
    } else if (m_ColorAttachements.empty()) {
        glDrawBuffer(GL_NONE);
    }

    dev::LOG->require(
        glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "Framebuffer status incomplete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
    glViewport(0, 0, m_Specification.size.x, m_Specification.size.y);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t Framebuffer::get_color_attachment_renderer_id(uint32_t index) const {
    dev::LOG->require(index < m_ColorAttachements.size(), "");
    return m_ColorAttachements[index];
}

void Framebuffer::resize(const math::ivec2& size) {
    m_Specification.size = size;
    invalidate();
}

int Framebuffer::read_pixel(uint32_t attachementIndex,
                            const math::ivec2& position) {
    dev::LOG->require(attachementIndex < m_ColorAttachements.size(), "");
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachementIndex);
    int pixelData;
    glReadPixels(position.x, position.y, 1, 1, GL_RED_INTEGER, GL_INT,
                 &pixelData);

    return pixelData;
}

void Framebuffer::clear_attachement(uint32_t attachementIndex, int value) {
    dev::LOG->require(attachementIndex < m_ColorAttachements.size(), "");

    auto& spec = m_ColorAttachementSpecifications[attachementIndex];
    glClearTexImage(m_ColorAttachements[attachementIndex], 0,
                    rurouni_fb_texture_format_to_gl(spec.TextureFormat), GL_INT,
                    &value);
}

void Framebuffer::draw_color_attachement(uint32_t index) {
    glBindFramebuffer(GL_FRAMEBUFFER, index);

    m_ScreenVA->bind();
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, get_color_attachment_renderer_id());
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace rr::graphics
