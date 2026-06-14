import json
import os.path
import shutil
import sys
from enum import Enum
from pickletools import optimize

from PIL import Image
import numpy as np

import CompressonatorAPI as Compressonator
from Common import *


class TextureType(Enum):
    BASE_COLOR = "BASE_COLOR"
    ORM = "ORM"
    NORMAL = "NORMAL"


class Texture:
    def __init__(self):
        self.texture_type: TextureType = TextureType.BASE_COLOR
        self.index: int = 0
        self.path: str = ""


def get_type_format(type: TextureType):
    if type in [TextureType.BASE_COLOR, TextureType.ORM]:
        return "BC1"
    elif type == TextureType.NORMAL:
        return "BC5"

    return "NONE"


def compress_gltf(
        gltf_path: str,
        output_dir: str = "",
        flip_normals: bool = False
):
    Compressonator.init()

    if len(output_dir) == 0:
        output_dir = os.path.join(os.path.dirname(gltf_path), "compressed")

    source_name = os.path.basename(gltf_path)
    source_dir = os.path.dirname(gltf_path)
    temp_dir = os.path.join(output_dir, "Temp")

    output_file = os.path.join(output_dir, source_name)

    if not os.path.isfile(gltf_path):
        print("ERROR: provided path does not exist")
        return -1

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    processed_textures = {}
    textures_redirectors = {}
    textures: list[Texture] = []

    print("Parsing GLTF")
    # Parse GLTF
    gltf = None
    with open(gltf_path, "r") as file:
        gltf = json.load(file)

    def append_texture(texture: Texture):
        if not texture.index in processed_textures:
            textures.append(texture)

    # extract texture data
    for material in gltf.get("materials"):
        if "pbrMetallicRoughness" in material:
            pbr_material = material["pbrMetallicRoughness"]

            if "baseColorTexture" in pbr_material:
                base_color = Texture()
                base_color.texture_type = TextureType.BASE_COLOR
                base_color.index = pbr_material["baseColorTexture"]["index"]
                append_texture(base_color)

            if "metallicRoughnessTexture" in pbr_material:
                orm_texture = Texture()
                orm_texture.texture_type = TextureType.ORM
                orm_texture.index = pbr_material["metallicRoughnessTexture"]["index"]
                append_texture(orm_texture)

        if "normalTexture" in material:
            normal_texture = Texture()
            normal_texture.texture_type = TextureType.NORMAL
            normal_texture.index = material["normalTexture"]["index"]

            append_texture(normal_texture)

    gltf_images_array = gltf["images"]
    for texture in textures:
        gltf_image = gltf_images_array[texture.index]
        texture.path = gltf_image["uri"]

    # flip normals if required
    if flip_normals:
        print("Flipping normals")
        os.makedirs(temp_dir, exist_ok=True)

        for texture in textures:
            if texture.texture_type == TextureType.NORMAL:
                file_name = os.path.basename(texture.path)
                file_path = os.path.join(temp_dir, file_name)
                texture_path = os.path.join(source_dir, texture.path)

                textures_redirectors[texture.path] = file_path

                if not os.path.exists(file_path):
                    print(f"Flipping {texture.path}")
                    src_image_data = np.asarray(Image.open(texture_path), copy=True)
                    src_image_data[:, :, 1] ^= 0xFF
                    image = Image.fromarray(src_image_data)
                    image.save(file_path, format="PNG", optimize=True, quality=95)
                else:
                    print(f"Flipping Cache Hit! {texture.path}")
                    continue


    print(f"Compressing Textures")
    for texture in textures:
        if os.path.splitext(texture.path)[1] != ".dds":
            target_path = os.path.join(output_dir, texture.path)
            target_path = os.path.splitext(target_path)[0] + ".dds"
            target_dir = os.path.dirname(target_path)
            destination_format = get_type_format(texture.texture_type)

            abs_source = None
            if texture.path in textures_redirectors:
                abs_source = textures_redirectors[texture.path]
            else:
                abs_source = os.path.join(source_dir, texture.path)

            if not os.path.exists(target_dir):
                os.makedirs(target_dir)

            Compressonator.compress(destination_format, abs_source, target_path)
            relative_target_path = os.path.relpath(target_path, output_dir)
            texture.path = relative_target_path

    print(f"Parsing saving modified GLTF to {output_dir}")
    # update gltf
    for texture in textures:
        gltf_image = gltf_images_array[texture.index]
        gltf_image["mimeType"] = "image/vnd-ms.dds"
        gltf_image["uri"] = texture.path

    with open(output_file, "w") as file:
        json.dump(gltf, file, indent=4)

    # copy binary
    binary_path = os.path.splitext(gltf_path)[0] + ".bin"
    shutil.copy(binary_path, output_dir)

    return output_file


if __name__ == "__main__":
    gltf = sys.argv[1]
    compress_gltf(sys.argv[1])
