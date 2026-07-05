import os
import platform
import subprocess
import tarfile
import zipfile
from urllib.request import urlretrieve

from Common import *

COMPRESSONATOR_CLI_VERSION = "4.5.52"
COMPRESSONATOR_CLI_DIR = f"{CACHE_DIR}/compressonatorcli"

COMPRESSONATOR_CLI_LINUX_ARCHIVE = f"compressonatorcli-{COMPRESSONATOR_CLI_VERSION}-Linux.tar.gz"
COMPRESSONATOR_CLI_LINUX_URL = f"https://github.com/GPUOpen-Tools/compressonator/releases/download/V{COMPRESSONATOR_CLI_VERSION}/{COMPRESSONATOR_CLI_LINUX_ARCHIVE}"
COMPRESSONATOR_CLI_LINUX = f"{COMPRESSONATOR_CLI_DIR}/compressonatorcli"

COMPRESSONATOR_CLI_WINDOWS_ARCHIVE = f"compressonatorcli-{COMPRESSONATOR_CLI_VERSION}-win64.zip"
COMPRESSONATOR_CLI_WINDOWS_URL = f"https://github.com/GPUOpen-Tools/compressonator/releases/download/V{COMPRESSONATOR_CLI_VERSION}/{COMPRESSONATOR_CLI_WINDOWS_ARCHIVE}"
COMPRESSONATOR_CLI_WINDOWS = f"{CACHE_DIR}/compressonatorcli-{COMPRESSONATOR_CLI_VERSION}-win64/compressonatorcli.exe"

def get_archive_name() -> str:
    if platform.system() == "Windows":
        return COMPRESSONATOR_CLI_WINDOWS_ARCHIVE

    return COMPRESSONATOR_CLI_LINUX_ARCHIVE

def get_archive_url() -> str:
    if platform.system() == "Windows":
        return COMPRESSONATOR_CLI_WINDOWS_URL

    return COMPRESSONATOR_CLI_LINUX_URL

def get_binary_name() -> str:
    if platform.system() == "Windows":
        return COMPRESSONATOR_CLI_WINDOWS

    return COMPRESSONATOR_CLI_LINUX

def fetch():
   if os.path.exists(COMPRESSONATOR_CLI_LINUX):
      return

   archive_path = os.path.join(CACHE_DIR, get_archive_name())

   if not os.path.exists(archive_path):
      print("Downloading compressonatorcli...")
      urlretrieve(get_archive_url(), archive_path, show_download_progress)

   if platform.system() == "Windows":
      with zipfile.ZipFile(archive_path) as archive_file:
         print("Uncompressing compressonatorcli...")
         archive_file.extractall(CACHE_DIR)
         # os.rename(archive_path[0:-len(".tar.gz")], COMPRESSONATOR_CLI_DIR)
   else:
      with tarfile.open(archive_path) as archive_file:
         print("Uncompressing compressonatorcli...")
         archive_file.extractall(CACHE_DIR)
         os.rename(archive_path[0:-len(".tar.gz")], COMPRESSONATOR_CLI_DIR)

   os.remove(archive_path)



def compress(dst_format: str, input: str, output: str):
    if os.path.exists(output):
        print(f"Output file ({output}) already exists.")
        return

    os.system(f"{get_binary_name()} -EncodeWith HPC -mipsize 4 -fd {dst_format} {os.path.abspath(input)} {os.path.abspath(output)}")

def init():
    # Create cache dir if it doesn't exist.
    if not os.path.exists(CACHE_DIR):
        os.mkdir(CACHE_DIR)

    if not os.path.exists(EXTERNAL_DIR):
        os.mkdir(EXTERNAL_DIR)

    fetch()
