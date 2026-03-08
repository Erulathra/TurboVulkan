# Turbo Vulkan Engine

There will be a Game engine written in C++ using Vulcan.

# Prerequisites

> The Intel's Sponza is too big to keep it on Git repo, so I decided to fetch it using a python script.
> Run `FetchAndCompressSponza.py` to fetch and then compress its textures using AMD's Compesonator.

> To fetch slang binaries you need to run a `FetchAndSlangBinaries.py` python script.

# GDB customizations:
If you want to use custom pretty printers, you need to add project path to GDB's auto-load safe-path. 
You can do that by adding `set auto-load safe-path <ProjectRoot>` to `~/.gdbinit`.