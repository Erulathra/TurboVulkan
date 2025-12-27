# Turbo Vulkan Engine

There will be a Game engine written in C++ using Vulcan.

# Prerequisites
`todo`

# GDB customizations:
If you want to use custom pretty printers, you need to add project path to GDB's auto-load safe-path. 
You can do that by adding `set auto-load safe-path <ProjectRoot>` to `~/.gdbinit`.

# CMake's command line arguments
* `-DBUILD_TYPE=Development/Shipping` -- Enables development or shipping configuration.
* `-DCMAKE_BUILD_TYPE=Debug/Release` -- Enables debug or release configuration.
