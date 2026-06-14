import os.path
import platform
import shutil
import tarfile
from urllib.request import urlretrieve

from Common import *

SLANG_VERSION="2026.4"

SLANG_URL="https://github.com/shader-slang/slang/releases/download/v{version}/slang-{version}-{platform}-x86_64.tar.gz"
SLANG_ARCHIVE="slang-{version}-{platform}-x86_64.tar.gz"

SLANG_PATH=os.path.join(CACHE_DIR, "slang-{version}-{platform}")

PLATFORM_LINUX="linux"

TARGET_SLANG=f"{PROJECT_DIR}/ThirdParty/Slang"

def fetch_slang(platform_name):
    archive_name = SLANG_ARCHIVE.format(version=SLANG_VERSION, platform=platform_name)
    archive_path = os.path.join(CACHE_DIR, archive_name)
    slang_path = SLANG_PATH.format(version=SLANG_VERSION, platform=platform_name)
    url = SLANG_URL.format(version=SLANG_VERSION, platform=platform_name)

    if not os.path.exists(slang_path):
        print(f"Downloading Slang ({platform_name})...")
        urlretrieve(url, archive_path, show_download_progress)

        with tarfile.open(archive_path) as archive_file:
            print(f"Uncompressing {archive_name}...")
            archive_file.extractall(slang_path)

        os.remove(archive_path)

def copy_binaries(platform_name):
    slang_path = SLANG_PATH.format(version=SLANG_VERSION, platform=platform_name)

    target_bin = os.path.join(TARGET_SLANG, "bin")
    target_lib = os.path.join(TARGET_SLANG, "lib")
    target_include = os.path.join(TARGET_SLANG, "include")

    paths_to_delete = [target_bin, target_lib, target_include]

    for path in paths_to_delete:
        if os.path.exists(path):
            shutil.rmtree(path)

    shutil.copytree(os.path.join(slang_path, "bin"), target_bin)
    shutil.copytree(os.path.join(slang_path, "lib"), target_lib)
    shutil.copytree(os.path.join(slang_path, "include"), target_include)

def main():
    # Create cache dir if it doesn't't exists.
    if not os.path.exists(CACHE_DIR):
        os.mkdir(CACHE_DIR)

    if platform.system() == "Linux":
        fetch_slang(PLATFORM_LINUX)
        copy_binaries(PLATFORM_LINUX)
    else:
        print("Unsupported platform")

if __name__ == "__main__":
    main()
