# Documentation
This is a running documentation about the planned requirements and state of
the project. At the current stage changes will be very frequent...

## C++
> Standard: `c++17`

## OS
I develop on Linux, so this is the primary target right now, but the project
structure will be kept to easily replace os dependent source files.

Currently supported:
- Linux

## System Dependencies

- CMake

rurouni::lib libraries link to some external libraries, which may have
additional dependencies, e.g. glfw needs some x11 libraries installed when
creating windows on a x-server. See the next section for those libraries.

## Library Dependencies
These libraries are fetched by FetchContent or present in the include tree,
so they don't need to be installed on the system.

Libraries may be exchanged or self implemented in the future, so this list is
subject to change. For now known libraries are used and wrapped for faster 
development.

- [cereal](https://github.com/USCiLab/cereal)
- [entt](https://github.com/skypjack/entt)
- [glad](https://github.com/Dav1dde/glad)
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [imgui](https://github.com/ocornut/imgui)
- [magic_enum](https://github.com/Neargye/magic_enum)
- [msdf-gen](https://github.com/Chlumsky/msdfgen)
- [msdf-atlas-gen](https://github.com/Chlumsky/msdf-atlas-gen)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
- [stduuid](https://github.com/mariusbancila/stduuid)

## Project Structure
See [targets.md](./md/targets.md) for details about the targets.

The rurouni project structure contains asset and scene specifications
as well as the project specification. They may be exported in binary
format in distribution builds, but have a human readable form for 
development.

```sh
.
├── fonts
│  ├── example_ttf_font.ttf
│  └── example_otf_font.otf
├── scenes
│  └── example_scene.json
├── shaders
│  ├── example_shader.fragment.glsl
│  └── example_shader.vertex.glsl
├── textures
│  └── example_texture.png
├── assets.json
└── project.json
```

The idea is that `rurouni::runtime` is just a splash screen (or at the
very beginning of development headless) that loads a selected project directory
and loads and executes the project according to the project.json file.

`rurouni::editor` will be able to create, inspect, modify and verify those
project directories, as well provide import functionalities to make asset
importing saver and easier.

Scenes are in a sense a frame for update and render functionalities.
Update will be handled by a scene-script (probably lua) and rendering by
Layers. Layers will be pre defined inside `ruronui::core`. I don't intent to make a general purpose engine like Unreal or Unity,
so the runtime will not link against additional libs containing user defined
Layers. Layers also won't be scriptable, I think. Reason is to force
a specific look and feel of the projects on the authors. But the scene behaviour
(game mechanics, entities, ai and so on) will be fully scriptable.

Storyline and dialogs are serialized as markdown with special syntax for
entity markers/linking.

I kind of want a node-view inside the editor to specify entity and dialog
relations.
