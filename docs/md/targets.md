# Targets
All targets provide an ALIAS in form `rurouni::<target-name>`

```sh
.
├── editor
├── runtime
└── libs
    ├── audio
    ├── common
    ├── core
    ├── graphics
    ├── scripting
    └── ui
```

## editor
> Type: `executable`  
> Target: `rurouni-editor`  
> Alias: `rurouni::editor`  
> Dependencies:
> - rurouni::libs
> - imgui

An editor application to view, develop, debug and manage rurouni's
project files. See docs about project structure for more insight about
rurouni's project system.

## runtime
> Type: `executable`  
> Target: `rurouni-runtime`  
> Alias: `rurouni::runtime`  
> Dependencies:
> - rurouni::libs

The runtime executable loading rurouni project files and running the
scripts.

## libs
> Type: `interface`  
> Target: `rurouni-libs`  
> Alias: `rurouni::libs`  

Interface target to all library targets inside sub directories. See 
tree view at the top for the list of included libraries.
