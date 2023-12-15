#include "rurouni/pch.hpp"

// rurouni
#include "rurouni/core/components/identifier.hpp"
#include "rurouni/core/components/orthographic_projection.hpp"
#include "rurouni/core/components/texture.hpp"
#include "rurouni/core/components/transform.hpp"
#include "rurouni/core/layer.hpp"
#include "rurouni/core/layers/grid_layer.hpp"
#include "rurouni/core/logger.hpp"
#include "rurouni/core/scene.hpp"
#include "rurouni/graphics/framebuffer.hpp"
#include "rurouni/graphics/render_api.hpp"

// external
#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"

namespace rr::core {

Scene::Scene(const math::ivec2& viewportSize_px) {
    m_SceneState.ViewportSize_px = viewportSize_px;
    // framebuffer init
    graphics::FramebufferSpecification framebufferSpec;
    framebufferSpec.size = viewportSize_px;
    framebufferSpec.attachements = {
        graphics::FramebufferTextureFormat::RGBA8,
        graphics::FramebufferTextureFormat::ENTITY_ID,
        graphics::FramebufferTextureFormat::DEPTH24STENCIL8};
    m_Framebuffer = std::make_unique<graphics::Framebuffer>(framebufferSpec);
}

Scene::~Scene() {}

void Scene::on_update(float dt) {}

void Scene::on_render(graphics::BatchRenderer& renderer) {
    m_Framebuffer->bind();

    graphics::api::set_clear_color({0.1f, 0.1f, 0.25f, 1.0f});
    graphics::api::clear();

    math::mat4 cameraTransform;
    math::mat4 cameraProjection;

    // rendering layers
    auto cameraComponents =
        m_Registry
            .try_get<components::Transform, components::OrthographicProjection>(
                m_SceneState.LayerCameraId);
    if (std::get<0>(cameraComponents) && std::get<1>(cameraComponents)) {
        cameraTransform = std::get<0>(cameraComponents)->get_transform();
        cameraProjection = std::get<1>(cameraComponents)->get_projection();

        renderer.begin(cameraProjection, cameraTransform);
        for (int i = 0; i < m_Layers.size(); i++) {
            m_Layers[i]->on_render(renderer, m_Registry, m_SceneState);
        }
        renderer.end();
    }

    // rendering overlays
    cameraComponents =
        m_Registry
            .try_get<components::Transform, components::OrthographicProjection>(
                m_SceneState.OverlayCameraId);
    if (std::get<0>(cameraComponents) && std::get<1>(cameraComponents)) {
        cameraTransform = std::get<0>(cameraComponents)->get_transform();
        cameraProjection = std::get<1>(cameraComponents)->get_projection();

        renderer.begin(cameraProjection, cameraTransform);
        for (int i = 0; i < m_Overlays.size(); i++) {
            m_Overlays[i]->on_render(renderer, m_Registry, m_SceneState);
        }
        renderer.end();

        // rendering debug layer
        if (m_DebugLayer != nullptr) {
            renderer.begin(cameraProjection, cameraTransform);
            m_DebugLayer->on_render(renderer, m_Registry, m_SceneState);
            renderer.end();
        }
    }

    m_Framebuffer->unbind();
}

void Scene::load_scene(const system::Path& filepath,
                       const math::ivec2& viewportSize_px) {
    std::ifstream is(filepath);
    {
        cereal::JSONInputArchive input(is);
        input(cereal::make_nvp("name", m_Name),
              cereal::make_nvp("layers", m_Layers),
              cereal::make_nvp("overlays", m_Overlays),
              cereal::make_nvp("scene_state", m_SceneState));

        input.setNextName("registry");
        input.startNode();
        entt::snapshot_loader{m_Registry}
            .get<entt::entity>(input)
            .get<components::Identifier>(input)
            .get<components::Transform>(input)
            .get<components::Texture>(input)
            .get<components::OrthographicProjection>(input);
        input.finishNode();
    }
    is.close();

    set_viewport_size(viewportSize_px);
}

void Scene::write_scene(std::optional<system::Path> filepath) {
    std::ofstream os(filepath ? filepath.value() : m_Filepath);
    {
        cereal::JSONOutputArchive out(os);
        out(cereal::make_nvp("name", m_Name),
            cereal::make_nvp("layers", m_Layers),
            cereal::make_nvp("overlays", m_Overlays),
            cereal::make_nvp("scene_state", m_SceneState));

        out.setNextName("registry");
        out.startNode();
        entt::snapshot{m_Registry}
            .get<entt::entity>(out)
            .get<components::Identifier>(out)
            .get<components::Transform>(out)
            .get<components::Texture>(out)
            .get<components::OrthographicProjection>(out);
        out.finishNode();
    }
    os.close();
}

void Scene::update_camera_data() {
    m_Registry.view<components::OrthographicProjection>().each(
        [this](entt::entity entity, auto& projection) {
            switch (projection.get_unit_type()) {
                case components::OrthographicProjection::UnitType::Cell:
                    projection.set_aspect_ratio(m_SceneState.ViewportSize_px);
                    projection.set_size(m_SceneState.CellCount.y);
                    break;
                case components::OrthographicProjection::UnitType::Pixel:
                    projection.set_aspect_ratio(m_SceneState.ViewportSize_px);
                    projection.set_size(m_SceneState.ViewportSize_px.y);
                    break;
                case components::OrthographicProjection::UnitType::Fixed:
                    projection.set_aspect_ratio(m_SceneState.ViewportSize_px);
                    break;
                default:
                    projection.set_aspect_ratio(m_SceneState.ViewportSize_px);
                    break;
            }
        });
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
    m_SceneState.ViewportSize_px = size;

    m_Framebuffer->resize(size);

    float aspectRatio = (float)size.x / (float)size.y;
    int32_t cellCountY = m_SceneState.CellCount.y;
    m_SceneState.CellCount =
        math::ivec2(std::ceil(cellCountY * aspectRatio), cellCountY);
    m_SceneState.CellSize_px = math::ivec2(size.y / m_SceneState.CellCount.y);

    update_camera_data();
}

}  // namespace rr::core
