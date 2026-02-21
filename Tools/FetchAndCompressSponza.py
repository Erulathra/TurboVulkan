import os.path
import platform
import tarfile
import zipfile
import time
import pathlib
import json

from urllib.request import urlretrieve

EXTERNAL_DIR = f"{os.path.dirname(__file__)}/../Content/External"
CACHE_DIR = f"{os.path.dirname(__file__)}/Cache"

COMPRESSONATOR_CLI_VERSION = "4.5.52"
COMPRESSONATOR_CLI_DIR = f"{CACHE_DIR}/compressonatorcli"

COMPRESSONATOR_CLI_LINUX_ARCHIVE = f"compressonatorcli-{COMPRESSONATOR_CLI_VERSION}-Linux.tar.gz"
COMPRESSONATOR_CLI_LINUX_URL = f"https://github.com/GPUOpen-Tools/compressonator/releases/download/V{COMPRESSONATOR_CLI_VERSION}/{COMPRESSONATOR_CLI_LINUX_ARCHIVE}"
COMPRESSONATOR_CLI_LINUX = f"{COMPRESSONATOR_CLI_DIR}/compressonatorcli"

SPONZA_BASE_URL = "https://cdrdv2.intel.com/v1/dl/getContent/830833"
SPONZA_BASE_ARCHIVE = f"{EXTERNAL_DIR}/LV_SponzaBase.zip"
SPONZA_BASE = f"{EXTERNAL_DIR}/main_sponza"
SPONZA_BASE_TEXTURES_SRC = f"{SPONZA_BASE}/textures"
SPONZA_BASE_TEXTURES_DST = f"{SPONZA_BASE}/textures_compressed"

SPONZA_BASE_GLTF_SRC = f"{SPONZA_BASE}/NewSponza_Main_glTF_003.gltf"
SPONZA_BASE_GLTF_DST = f"{SPONZA_BASE}/SponzaCompressed.gltf"

last_report_time = 0


def show_download_progress(block_num, block_size, total_size):
    global last_report_time

    if time.perf_counter() - last_report_time > 1.:
        downloaded_mib = block_num * block_size / 2. ** 20
        total_mib = total_size / 2. ** 20
        print(f"Downloading {downloaded_mib / total_mib * 100.:.2f}% [{downloaded_mib:.2f}MiB/{total_mib:.2f}MiB]")
        last_report_time = time.perf_counter()


def fetch_compressonator():
    if platform.system() == "Linux":
        if os.path.exists(COMPRESSONATOR_CLI_LINUX):
            return

        archive_path = os.path.join(CACHE_DIR, COMPRESSONATOR_CLI_LINUX_ARCHIVE)

        if not os.path.exists(archive_path):
            print("Downloading compressonatorcli...")
            urlretrieve(COMPRESSONATOR_CLI_LINUX_URL, archive_path, show_download_progress)

        with tarfile.open(archive_path) as archiveFile:
            print("Uncompressing compressonatorcli...")
            archiveFile.extractall(CACHE_DIR)
            os.rename(archive_path[0:-len(".tar.gz")], COMPRESSONATOR_CLI_DIR)

        os.remove(archive_path)

    else:
        print("TODO: Implement for windows")


def fetch_sponza():
    if os.path.exists(SPONZA_BASE):
        return

    print("Downloading Intel's Sponza (base)...")
    urlretrieve(SPONZA_BASE_URL, SPONZA_BASE_ARCHIVE, show_download_progress)

    with zipfile.ZipFile(SPONZA_BASE_ARCHIVE, "r") as sponzaArchive:
        print("Extracting Intel's Sponza (base)...")
        sponzaArchive.extractall(os.path.join(SPONZA_BASE, ".."))

    os.remove(SPONZA_BASE_ARCHIVE)


def group_textures_by_type(texture_files: list[str]):
    result = {
        "BaseColor": [],
        "Metalness": [],
        "Normal": [],
        "Roughness": [],
        "CombinedRoughnessAndMetalness": [],
        "Opacity": []
    }

    for texture in texture_files:
        if "Roughness" in texture and "Metalness" in texture:
            result["CombinedRoughnessAndMetalness"].append(texture)
            continue

        for type in result.keys():
            if type in texture:
                result[type].append(texture)
                break

    return result


def run_compressonator(dst_format: str, input: str, output: str):
    if os.path.exists(output):
        return

    os.system(f"{COMPRESSONATOR_CLI_LINUX} -EncodeWith HPC -mipsize 4 -fd {dst_format} {os.path.abspath(input)} {os.path.abspath(output)}")


def get_dst_texture_path(src_path):
    base_name = pathlib.Path(src_path).stem + ".dds"
    return os.path.join(SPONZA_BASE_TEXTURES_DST, base_name)


def convert_textures(grouped_textures):
    if not os.path.exists(SPONZA_BASE_TEXTURES_DST):
        os.mkdir(SPONZA_BASE_TEXTURES_DST)

    for base_color in grouped_textures["BaseColor"]:
        run_compressonator("BC1", os.path.join(SPONZA_BASE_TEXTURES_SRC, base_color), get_dst_texture_path(base_color))

    for metalness in grouped_textures["Metalness"]:
        run_compressonator("BC4", os.path.join(SPONZA_BASE_TEXTURES_SRC, metalness), get_dst_texture_path(metalness))

    for roughness in grouped_textures["Roughness"]:
        run_compressonator("BC4", os.path.join(SPONZA_BASE_TEXTURES_SRC, roughness), get_dst_texture_path(roughness))

    for combined in grouped_textures["CombinedRoughnessAndMetalness"]:
        run_compressonator("BC1", os.path.join(SPONZA_BASE_TEXTURES_SRC, combined), get_dst_texture_path(combined))

    for opacity in grouped_textures["Opacity"]:
        run_compressonator("BC4", os.path.join(SPONZA_BASE_TEXTURES_SRC, opacity), get_dst_texture_path(opacity))

    for normal in grouped_textures["Normal"]:
        run_compressonator("BC5", os.path.join(SPONZA_BASE_TEXTURES_SRC, normal), get_dst_texture_path(normal))


def convert_gltf():
    with open(SPONZA_BASE_GLTF_SRC, 'r') as f:
        gltf = json.load(f)

    for image in gltf["images"]:
        compressedTexturePath = get_dst_texture_path(image["uri"])
        relative_path = os.path.relpath(compressedTexturePath, SPONZA_BASE)
        image["uri"] = relative_path
        image["mimeType"] = "image/vnd-ms.dds"

    with open(SPONZA_BASE_GLTF_DST, 'w') as f:
        json.dump(gltf, f, indent=4)


def main():
    # Create cache dir if it doesn't't exists.
    if not os.path.exists(CACHE_DIR):
        os.mkdir(CACHE_DIR)

    if not os.path.exists(EXTERNAL_DIR):
        os.mkdir(EXTERNAL_DIR)

    fetch_compressonator()
    fetch_sponza()

    texture_files = os.listdir(SPONZA_BASE_TEXTURES_SRC)
    grouped_textures = group_textures_by_type(texture_files)

    convert_textures(grouped_textures)
    convert_gltf()


if __name__ == "__main__":
    main()
