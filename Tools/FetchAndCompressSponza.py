import json
import os.path
import zipfile
from urllib.request import urlretrieve

import CompressGLTF
import CompressonatorAPI as Compressonator
from Common import *

SPONZA_BASE_URL = "https://cdrdv2.intel.com/v1/dl/getContent/830833"
SPONZA_BASE_ARCHIVE = f"{EXTERNAL_DIR}/LV_SponzaBase.zip"
SPONZA_BASE = f"{EXTERNAL_DIR}/main_sponza"

SPONZA_BASE_GLTF_SRC = f"{SPONZA_BASE}/NewSponza_Main_glTF_003.gltf"


def fetch_sponza():
    if os.path.exists(SPONZA_BASE):
        return

    print("Downloading Intel's Sponza (base)...")
    urlretrieve(SPONZA_BASE_URL, SPONZA_BASE_ARCHIVE, show_download_progress)

    with zipfile.ZipFile(SPONZA_BASE_ARCHIVE, "r") as sponzaArchive:
        print("Extracting Intel's Sponza (base)...")
        sponzaArchive.extractall(os.path.join(SPONZA_BASE, ".."))

    os.remove(SPONZA_BASE_ARCHIVE)


def main():
    Compressonator.init()
    fetch_sponza()

    compressed_gltf = CompressGLTF.compress_gltf(SPONZA_BASE_GLTF_SRC)

    gltf = None
    with open(compressed_gltf, 'r') as file:
        gltf = json.load(file)

    for light in gltf["extensions"]["KHR_lights_punctual"]["lights"]:
        if light["type"] == "point":
            light["intensity"] = 15.
            light["range"] = 10.

    with open(compressed_gltf, 'w') as file:
        json.dump(gltf, file, indent=4)


if __name__ == "__main__":
    main()
