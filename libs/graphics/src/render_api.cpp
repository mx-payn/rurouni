// pch
#include "rurouni/pch.hpp"
//-----------------------

// rurouni
#include "rurouni/graphics/logger.hpp"
#include "rurouni/graphics/render_api.hpp"
#include "rurouni/math.hpp"

// external
#include <glad/gl.h>

namespace rr::graphics::api {

static bool s_OpenGLInitialized = false;

static void opengl_message_callback(unsigned source,
                                    unsigned type,
                                    unsigned id,
                                    unsigned severity,
                                    int length,
                                    const char* message,
                                    const void* userParam) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            critical("{}", message);
            return;
        case GL_DEBUG_SEVERITY_MEDIUM:
            error("{}", message);
            return;
        case GL_DEBUG_SEVERITY_LOW:
            warn("{}", message);
            return;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            info("{}", message);
            return;
    }

    require(false, "Unknown severity level!");
}

void init() {
    if (s_OpenGLInitialized)
        return;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_message_callback, nullptr);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    s_OpenGLInitialized = true;
}

void bind_default_framebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void set_clear_color(const math::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void set_viewport(const math::ivec2& size) {
    glViewport(0, 0, size.x, size.y);
}

void draw_lines(std::shared_ptr<VertexArray> vertexArray, int32_t vertexCount) {
    vertexArray->bind();
    glEnable(GL_DEPTH_TEST);
    glDrawArrays(GL_LINES, 0, vertexCount);
}

void draw_indexed(std::shared_ptr<VertexArray> vertexArray,
                  int32_t indexCount) {
    int32_t count =
        indexCount ? indexCount : vertexArray->get_index_buffer()->get_count();
    vertexArray->bind();
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

}  // namespace rr::graphics::api
