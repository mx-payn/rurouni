#include "rurouni/pch.hpp"

// rurouni
#include "rurouni/core/components/identifier.hpp"
#include "rurouni/core/components/orthographic_projection.hpp"
#include "rurouni/core/components/transform.hpp"
#include "rurouni/core/layer.hpp"
#include "rurouni/core/layers/grid_layer.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/graphics/framebuffer.hpp"
#include "rurouni/graphics/render_api.hpp"

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

    // TEMP
    entt::entity camera = m_Registry.create();
    m_LayerCameraId = camera;
    m_Registry.emplace<components::Identifier>(camera, UUID::create(),
                                               "camera");
    auto& transform = m_Registry.emplace<components::Transform>(camera);
    transform.set_translation(transform.get_translation() -
                              math::vec3{0.5f, 0.5f, 0.0f});
    m_Registry.emplace<components::OrthographicProjection>(
        camera, components::OrthographicProjection::UnitType::Cell,
        m_ViewportSize_px, static_cast<float>(m_GridState.CellCount.y), -32.0f,
        32.0f);

    // set initial camera data based on set type
    m_Registry.view<components::OrthographicProjection>().each(
        [this](entt::entity entity, auto& projection) {
            switch (projection.get_unit_type()) {
                case components::OrthographicProjection::UnitType::Cell:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    projection.set_size(m_GridState.CellCount.y);
                    break;
                case components::OrthographicProjection::UnitType::Pixel:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    projection.set_size(m_ViewportSize_px.y);
                    break;
                case components::OrthographicProjection::UnitType::Fixed:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    break;
                default:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    break;
            }
        });
}

Scene::~Scene() {}

void Scene::on_update(float dt) {}

void Scene::on_render(graphics::BatchRenderer& renderer) {
    m_Framebuffer->bind();

    graphics::api::set_clear_color({0.1f, 0.1f, 0.25f, 1.0f});
    graphics::api::clear();

    math::mat4 cameraTransform;
    math::mat4 cameraProjection;

    // TODO m_Registry.try_get() might be a move here
    // rendering layers
    if (m_LayerCameraId != entt::null && m_Registry.valid(m_LayerCameraId) &&
        m_Registry
            .all_of<components::Transform, components::OrthographicProjection>(
                m_LayerCameraId)) {
        cameraTransform = m_Registry.get<components::Transform>(m_LayerCameraId)
                              .get_transform();
        cameraProjection =
            m_Registry.get<components::OrthographicProjection>(m_LayerCameraId)
                .get_projection();

        renderer.begin(cameraProjection, cameraTransform);
        for (int i = 0; i < m_Layers.size(); i++) {
            m_Layers[i]->on_render(renderer, m_Registry, m_GridState);
        }
        renderer.end();
    }

    // rendering overlays
    if (m_OverlayCameraId != entt::null &&
        m_Registry.valid(m_OverlayCameraId) &&
        m_Registry
            .all_of<components::Transform, components::OrthographicProjection>(
                m_OverlayCameraId)) {
        cameraTransform =
            m_Registry.get<components::Transform>(m_OverlayCameraId)
                .get_transform();
        cameraProjection =
            m_Registry
                .get<components::OrthographicProjection>(m_OverlayCameraId)
                .get_projection();

        renderer.begin(cameraProjection, cameraTransform);
        for (int i = 0; i < m_Overlays.size(); i++) {
            m_Overlays[i]->on_render(renderer, m_Registry, m_GridState);
        }
        renderer.end();
    }

    // rendering debug layer
    if (m_DebugLayer != nullptr && m_OverlayCameraId != entt::null &&
        m_Registry.valid(m_OverlayCameraId) &&
        m_Registry
            .all_of<components::Transform, components::OrthographicProjection>(
                m_OverlayCameraId)) {
        cameraTransform =
            m_Registry.get<components::Transform>(m_OverlayCameraId)
                .get_transform();
        cameraProjection =
            m_Registry
                .get<components::OrthographicProjection>(m_OverlayCameraId)
                .get_projection();

        renderer.begin(cameraProjection, cameraTransform);
        m_DebugLayer->on_render(renderer, m_Registry, m_GridState);
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

    m_Registry.view<components::OrthographicProjection>().each(
        [this, size](entt::entity entity, auto& projection) {
            switch (projection.get_unit_type()) {
                case components::OrthographicProjection::UnitType::Cell:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    projection.set_size(m_GridState.CellCount.y);
                    break;
                case components::OrthographicProjection::UnitType::Pixel:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    projection.set_size(m_ViewportSize_px.y);
                    break;
                case components::OrthographicProjection::UnitType::Fixed:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    break;
                default:
                    projection.set_aspect_ratio(m_ViewportSize_px);
                    break;
            }
        });
}

}  // namespace rr::core
