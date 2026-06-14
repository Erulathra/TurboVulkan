import os
import platform
import subprocess
import tarfile
from urllib.request import urlretrieve

from Common import *

COMPRESSONATOR_CLI_VERSION = "4.5.52"
COMPRESSONATOR_CLI_DIR = f"{CACHE_DIR}/compressonatorcli"

COMPRESSONATOR_CLI_LINUX_ARCHIVE = f"compressonatorcli-{COMPRESSONATOR_CLI_VERSION}-Linux.tar.gz"
COMPRESSONATOR_CLI_LINUX_URL = f"https://github.com/GPUOpen-Tools/compressonator/releases/download/V{COMPRESSONATOR_CLI_VERSION}/{COMPRESSONATOR_CLI_LINUX_ARCHIVE}"
COMPRESSONATOR_CLI_LINUX = f"{COMPRESSONATOR_CLI_DIR}/compressonatorcli"

def fetch():
    if platform.system() == "Linux":
        if os.path.exists(COMPRESSONATOR_CLI_LINUX):
            return

        archive_path = os.path.join(CACHE_DIR, COMPRESSONATOR_CLI_LINUX_ARCHIVE)

        if not os.path.exists(archive_path):
            print("Downloading compressonatorcli...")
            urlretrieve(COMPRESSONATOR_CLI_LINUX_URL, archive_path, show_download_progress)

        with tarfile.open(archive_path) as archive_file:
            print("Uncompressing compressonatorcli...")
            archive_file.extractall(CACHE_DIR)
            os.rename(archive_path[0:-len(".tar.gz")], COMPRESSONATOR_CLI_DIR)

        os.remove(archive_path)

    else:
        print("TODO: Implement for windows")


def compress(dst_format: str, input: str, output: str):
    if os.path.exists(output):
        print(f"Output file ({output}) already exists.")
        return

    os.system(f"{COMPRESSONATOR_CLI_LINUX} -EncodeWith HPC -mipsize 4 -fd {dst_format} {os.path.abspath(input)} {os.path.abspath(output)}")

def init():
    # Create cache dir if it doesn't exist.
    if not os.path.exists(CACHE_DIR):
        os.mkdir(CACHE_DIR)

    if not os.path.exists(EXTERNAL_DIR):
        os.mkdir(EXTERNAL_DIR)

    fetch()
