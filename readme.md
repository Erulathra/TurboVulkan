# Turbo Vulkan Engine

There will be a Game engine written in C++ using Vulcan.

## Prerequisites

> The Intel's Sponza is too big to keep it on Git repo, so I decided to fetch it using a python script.
> Run `FetchAndCompressSponza.py` to fetch and then compress its textures using AMD's Compesonator.

> To fetch slang binaries you need to run a `FetchAndSlangBinaries.py` python script.

## GDB customizations:
If you want to use custom pretty printers, you need to add project path to GDB's auto-load safe-path. 
You can do that by adding `set auto-load safe-path <ProjectRoot>` to `~/.gdbinit`.

## Building
There are three build configurations: `Development`, `Test` and `Shipping`.
All of the can be used with all CMake ones such as `debug` or `release`.

For now project only supports CLANG compiler, and for the full zed support you need to add mingw-clang binaries
to your PATH.

Bellow is example of building the `Debug Development` configuration:
```bash
cmake -S . -B cmake-build-debug-development -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TYPE=Development
```
