#include "rurouni/graphics/framebuffer.hpp"
#include "rurouni/pch.hpp"

#include "rurouni/core/layer.hpp"
#include "rurouni/core/layers/grid_layer.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/graphics/render_api.hpp"

#include <memory>

namespace rr::core {

const int32_t CELL_COUNT_Y = 45;

Scene::Scene(const math::ivec2& viewportSize_px)
    : m_ViewportSize_px(viewportSize_px) {
    // framebuffer init
    graphics::FramebufferSpecification framebufferSpec;
    framebufferSpec.size = viewportSize_px;
    framebufferSpec.attachements = {
        graphics::FramebufferTextureFormat::RGBA8,
        graphics::FramebufferTextureFormat::ENTITY_ID,
        graphics::FramebufferTextureFormat::DEPTH24STENCIL8};
    m_Framebuffer = std::make_unique<graphics::Framebuffer>(framebufferSpec);

    // grid state init
    float aspectRatio = (float)viewportSize_px.x / (float)viewportSize_px.y;
    m_GridState.CellCount =
        math::ivec2(std::floor(CELL_COUNT_Y * aspectRatio), CELL_COUNT_Y);
    m_GridState.CellSize_px =
        math::ivec2(viewportSize_px.y / m_GridState.CellCount.y);

    push_layer(std::make_unique<GridLayer>(m_GridState));

    // TODO debug
    // camera
    m_CameraTransform = math::translate(math::mat4(1.0f), math::vec3(0.0f));
    m_CameraProjection = math::ortho(
        0.0f, static_cast<float>(m_GridState.CellCount.x - 1),
        static_cast<float>(m_GridState.CellCount.y - 1), 0.0f, -64.0f, 64.0f);
}

Scene::~Scene() {}

void Scene::on_update(float dt) {}

void Scene::on_render(graphics::BatchRenderer& renderer) {
    m_Framebuffer->bind();

    graphics::api::set_clear_color({0.1f, 0.1f, 0.25f, 1.0f});
    graphics::api::clear();

    // rendering layers
    renderer.begin(m_CameraProjection, m_CameraTransform);
    for (int i = 0; i < m_Layers.size(); i++) {
        m_Layers[i]->on_render(renderer, m_GridState);
    }
    renderer.end();

    // rendering overlays
    renderer.begin(m_CameraProjection, m_CameraTransform);
    for (int i = 0; i < m_Overlays.size(); i++) {
        m_Overlays[i]->on_render(renderer, m_GridState);
    }
    renderer.end();

    // rendering debug layer
    if (m_DebugLayer != nullptr) {
        renderer.begin(m_CameraProjection, m_CameraTransform);
        m_DebugLayer->on_render(renderer, m_GridState);
        renderer.end();
    }

    m_Framebuffer->unbind();
}

void Scene::push_layer(std::unique_ptr<Layer> layer) {
    layer->on_attach();
    m_Layers.push_back(std::move(layer));
}

void Scene::pop_layer() {
    m_Layers.back()->on_detach();
    m_Layers.pop_back();
}

void Scene::push_overlay(std::unique_ptr<Layer> overlay) {
    overlay->on_attach();
    m_Overlays.push_back(std::move(overlay));
}

void Scene::pop_overlay() {
    m_Overlays.back()->on_detach();
    m_Overlays.pop_back();
}

void Scene::set_debug_layer(std::shared_ptr<Layer> layer) {
    if (layer == nullptr) {
        m_DebugLayer->on_detach();
        m_DebugLayer = nullptr;
        return;
    }

    if (m_DebugLayer != nullptr)
        m_DebugLayer->on_detach();

    m_DebugLayer = layer;
    m_DebugLayer->on_attach();
}

void Scene::set_viewport_size(const math::ivec2& size) {
    m_ViewportSize_px = size;

    m_Framebuffer->resize(size);

    float aspectRatio = (float)m_ViewportSize_px.x / (float)m_ViewportSize_px.y;
    m_GridState.CellCount =
        math::ivec2(std::floor(CELL_COUNT_Y * aspectRatio), CELL_COUNT_Y);
    m_GridState.CellSize_px =
        math::ivec2(m_ViewportSize_px.y / m_GridState.CellCount.y);

    m_CameraTransform = math::translate(math::mat4(1.0f), math::vec3(0.0f));
    m_CameraProjection = math::ortho(
        0.0f, static_cast<float>(m_GridState.CellCount.x - 1),
        static_cast<float>(m_GridState.CellCount.y - 1), 0.0f, -64.0f, 64.0f);
}
}  // namespace rr::core
