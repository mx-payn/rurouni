#ifndef RR_EDITOR_UI_H
#define RR_EDITOR_UI_H

#include "imgui/imgui_internal.h"
#include "msdf-atlas-gen/types.h"
#include "rurouni/core/scene.hpp"
#include "rurouni/editor/state.hpp"
#include "rurouni/graphics/font.hpp"
#include "rurouni/graphics/window.hpp"
#include "rurouni/system/filesystem.hpp"
#include "rurouni/types/uuid.hpp"

#include <imgui/imgui.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace rr::editor::ui {

struct FontImportSpecification {
    // font import data
    system::Path FontFilepath;
    system::Path CharsetFilepath = system::Path();
    bool VariableFont;
    msdf_atlas::GlyphIdentifierType GlyphIdentifierType =
        msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT;
    double FontScale = -1;
    std::string FontName;
};

#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define DEFAULT_MITER_LIMIT 1.0
#define DEFAULT_PIXEL_RANGE 2.0
#define SDF_ERROR_ESTIMATE_PRECISION 19
#define GLYPH_FILL_RULE msdfgen::FILL_NONZERO

struct FontImportConfiguration {
    // font specification data
    msdf_atlas::ImageType ImageType = msdf_atlas::ImageType::MSDF;
    msdf_atlas::ImageFormat ImageFormat = msdf_atlas::ImageFormat::UNSPECIFIED;
    msdf_atlas::YDirection YDirection = msdf_atlas::YDirection::BOTTOM_UP;
    int Width, Height;
    double EmSize;
    double PxRange;
    double AngleThreshold = DEFAULT_ANGLE_THRESHOLD;
    double MiterLimit = DEFAULT_MITER_LIMIT;
    void (*EdgeColoring)(msdfgen::Shape&,
                         double,
                         unsigned long long) = msdfgen::edgeColoringInkTrap;
    bool ExpensiveColoring = false;
    unsigned long long ColoringSeed;
    msdf_atlas::GeneratorAttributes GeneratorAttributes;
    bool PreprocessGeometry = false;
    bool Kerning = true;
    int ThreadCount = 0;
    system::Path ImageFilepath;
    std::vector<graphics::unicode_t> Charset;

    // not used atm
    // const char *arteryFontFilename;
    // const char *jsonFilename;
    // const char *csvFilename;
    // const char *shadronPreviewFilename;
    // const char *shadronPreviewText;
};

void init(graphics::Window& window,
          const system::Path& sharedDataDir,
          const system::Path& userDataDir,
          const system::Path& userConfigDir);
void terminate();
void set_display_size(const glm::ivec2 size);
void update(float dt, const system::Path& userConfigDir);
void begin();
void end();

void show_dockspace();

/////////////////////////////
//  PANELS
void show_imgui_demo();
void show_startup_splash(
    std::unordered_map<UUID, ModuleHistoryItem>& history,
    std::function<bool(const system::Path&)> importModuleFunc,
    std::function<bool(const UUID&)> openModuleFunc,
    std::function<bool(const system::Path&, const std::string&)>
        createModuleFunc,
    std::function<void()> exitFunc);
void show_properties(entt::registry& registry,
                     core::AssetManager& assetManager);
void show_asset_manager(
    core::AssetManager& assetManager,
    std::function<bool(core::TextureSpecification&)> importTextureFunc,
    std::function<bool(std::unordered_map<int, core::SpriteSpecification>&)>
        importSpritesFunc,
    std::function<bool(core::ShaderSpecification&)> importShaderFunc,
    std::function<bool(ui::FontImportSpecification&,
                       ui::FontImportConfiguration&)> importFontFunc);
void show_scene_manager(core::Scene& scene,
                        std::function<void(const system::Path&)> changeSceneFn,
                        std::function<void(const system::Path&)> saveSceneFn);
void show_scene_viewport(
    core::Scene& scene,
    std::function<void()> drawSceneFn,
    std::function<bool(const system::Path&)> changeSceneFn);
// TODO
void show_module_manager();
void show_file_browser(const system::Path& rootDir);
void show_toasts();

/////////////////////////////
//  MODALS
void show_create_module_modal(
    std::function<bool(const system::Path&, const std::string&)>
        createModuleFunc);
void show_import_module_modal(
    std::function<bool(const system::Path&)> importModuleFunc);
void show_import_assets_modal(
    core::AssetManager& assetManager,
    std::function<bool(core::TextureSpecification&)> importTextureFunc,
    std::function<bool(std::unordered_map<int, core::SpriteSpecification>&)>
        importSpritesFunc,
    std::function<bool(core::ShaderSpecification&)> importShaderFunc,
    std::function<bool(ui::FontImportSpecification&,
                       ui::FontImportConfiguration&)> importFontFunc);

/////////////////////////////
//  IMPORT ASSETS TABS
void show_import_assets_tab_texture(
    core::AssetManager& assetManager,
    std::function<bool(core::TextureSpecification&)> importTextureFunc);
void show_import_assets_tab_sprite(
    core::AssetManager& assetManager,
    std::function<bool(std::unordered_map<int, core::SpriteSpecification>&)>
        importSpritesFunc);
void show_import_assets_tab_font(
    core::AssetManager& assetManager,
    std::function<bool(ui::FontImportSpecification&,
                       ui::FontImportConfiguration&)> importFontFunc);
void show_import_assets_tab_shader(
    core::AssetManager& assetManager,
    std::function<bool(core::ShaderSpecification&)> importShaderFunc);

/////////////////////////////
//  PROPERTY TABS
void show_property_tab_entity(entt::entity entity,
                              entt::registry& registry,
                              core::AssetManager& assetManager);
void show_property_tab_texture(const UUID& id,
                               core::AssetManager& assetManager);
void show_property_tab_sprite(const UUID& id, core::AssetManager& assetManager);
void show_property_tab_font(const UUID& id, core::AssetManager& assetManager);
// TODO
void show_property_tab_shader();

/////////////////////////////
//  TOAST
enum class ToastType { None, Info, Warning, Error, Success };
enum class ToastPhase { FadeIn, Display, FadeOut, Finished };
enum class ToastPosition { TopLeft, TopRight, BottomLeft, BottomRight };
struct Toast {
    ToastType Type;
    UUID Id = UUID::create();
    std::string Title = std::string();
    std::string Message = std::string();

    float StartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();

    Toast(ToastType type = ToastType::None) : Type(type) {}
    template <typename... Args>
    Toast(ToastType type,
          const std::string& title,
          fmt::format_string<Args...> messageFmt,
          Args&&... args)
        : Type(type),
          Title(title),
          Message(fmt::format(messageFmt, std::forward<Args>(args)...)) {}
};
void push_toast(const Toast& toast);

/////////////////////////////
//  PROPERTY
enum class PropertyTabType { Entity, Texture, Sprite, Font, Shader };
struct PropertyTab {
    PropertyTabType Type;
    std::string Name;
    UUID Id;

    union data_t {
        entt::entity Entity;
        UUID AssetId;

        data_t(entt::entity entity) : Entity(entity) {}
        data_t(const UUID& id) : AssetId(id) {}
    } Data;

    PropertyTab(PropertyTabType type,
                const std::string& name,
                const data_t& data,
                const UUID& id = UUID::create())
        : Type(type), Name(name), Data(data), Id(id) {}
};
void push_property_tab(const PropertyTab& tab);
void remove_property_tab(PropertyTabType type, const PropertyTab::data_t& data);

struct RRUIToastTypeStyle {
    //////////////////////////////////
    /////// ANIMATION
    float DisplayTime = 4000.0f;
    float FadeInTime = 150.0f;
    float FadeOutTime = 450.0f;

    //////////////////////////////////
    /////// COLORS
    ImVec4 TitleColor =
        ImVec4(243.0f / 255.0f, 248.0f / 255.0f, 255.0f / 255.0f, 1.0f);
    ImVec4 MessageColor =
        ImVec4(243.0f / 255.0f, 248.0f / 255.0f, 255.0f / 255.0f, 1.0f);
    ImVec4 SeparatorColor =
        ImVec4(243.0f / 255.0f, 248.0f / 255.0f, 255.0f / 255.0f, 1.0f);
    ImVec4 BackgroundColor =
        ImVec4(1.0f / 255.0f, 30.0f / 255.0f, 50.0f / 255.0f, 1.0f);
    ImVec4 IconColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    //////////////////////////////////
    /////// ICON
    std::optional<const char*> IconCodepoint = std::nullopt;
};

struct RRUIStyle {
    float FontSize = 15.0f;

    ImVec2 ThumbnailSize = {128.0f, 128.0f};
    ImVec2 ThumbnailPadding = {16.0f, 16.0f};

    ImVec2 IconSize = {32.0f, 32.0f};
    ImVec2 IconPadding = {4.0f, 4.0f};

    ToastPosition ToastViewportPosition = ToastPosition::TopRight;
    float ToastBorderRounding = 3.0f;
    float ToastBorderSize = 0.0f;
    ImVec2 ToastPadding = {20.0f, 20.0f};
    float ToastMaxWidth = 550.0f;
    float ToastMinWidth = 350.0f;
    ImVec2 ToastContentPadding = {10.0f, 10.0f};
    bool ToastContentSeparator = true;
    ImFont* ToastIconFont = nullptr;
    ImFont* ToastContentFont = nullptr;

    std::unordered_map<ToastType, RRUIToastTypeStyle> ToastTypeStyle;
};

struct RRUIContext {
    bool ShowStartupSplash = true;
    bool ShowFileBrowser = true;
    bool ShowAssetManager = true;
    bool ShowSceneManager = true;
    bool ShowProperties = true;
    bool ShowSceneViewport = true;
    bool ShowImGuiDemo = false;

    std::optional<entt::entity> SelectedEntity;
    std::optional<UUID> SelectedPropertyTabId;

    math::ivec2 SceneViewportSize = math::ivec2(1.0f);

    struct {
        std::unique_ptr<graphics::Texture> File;
        std::unique_ptr<graphics::Texture> FileImport;
        std::unique_ptr<graphics::Texture> FileOpen;
        std::unique_ptr<graphics::Texture> Folder;
        std::unique_ptr<graphics::Texture> Save;
        std::unique_ptr<graphics::Texture> SquarePlus;
    } Icons;

    struct {
        ImFont* Monospace_Regular;
        ImFont* Monospace_Bold;
        ImFont* Monospace_Italic;
        ImFont* Icon;
    } Fonts;

    RRUIStyle Style;

    std::vector<Toast> Toasts;
    std::vector<PropertyTab> PropertyTabs;
};

RRUIContext& get_context();

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_H
