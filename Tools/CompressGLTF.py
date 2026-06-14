import json
import os.path
import shutil
import sys
from enum import Enum

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

    def get_type_format(self):
        if self.texture_type in [TextureType.BASE_COLOR, TextureType.ORM]:
            return "BC1"
        elif self.texture_type == TextureType.NORMAL:
            return "BC5"

        return "NONE"


def compress_gltf(gltf_path :str, output_dir :str = ""):
    Compressonator.init()

    source_name = os.path.basename(gltf_path)
    source_dir = os.path.dirname(gltf_path)

    if len(output_dir) == 0:
        output_dir = os.path.join(os.path.dirname(gltf_path), "compressed")

    output_file = os.path.join(output_dir, source_name)

    if not os.path.isfile(gltf_path):
        print("ERROR: provided path does not exist")
        return -1

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    textures: list[Texture] = []

    print("Parsing GLTF")
    # Parse GLTF
    gltf = None
    with open(gltf_path, "r") as file:
        gltf = json.load(file)

    # extract texture data
    for material in gltf.get("materials"):
        if "pbrMetallicRoughness" in material:
            pbr_material = material["pbrMetallicRoughness"]

            if "baseColorTexture" in pbr_material:
                base_color = Texture()
                base_color.texture_type = TextureType.BASE_COLOR
                base_color.index = pbr_material["baseColorTexture"]["index"]
                textures.append(base_color)

            if "metallicRoughnessTexture" in pbr_material:
                orm_texture = Texture()
                orm_texture.texture_type = TextureType.ORM
                orm_texture.index = pbr_material["metallicRoughnessTexture"]["index"]
                textures.append(orm_texture)

        if "normalTexture" in material:
            normal_texture = Texture()
            normal_texture.texture_type = TextureType.NORMAL
            normal_texture.index = material["normalTexture"]["index"]

            textures.append(normal_texture)

    gltf_images_array = gltf["images"]
    for texture in textures:
        gltf_image = gltf_images_array[texture.index]
        texture.path = gltf_image["uri"]

    print(f"Compressing Textures")
    for texture in textures:
        if os.path.splitext(texture.path)[1] != ".dds":
            target_path = os.path.join(output_dir, texture.path)
            target_path = os.path.splitext(target_path)[0] + ".dds"
            target_dir = os.path.dirname(target_path)
            destination_format = texture.get_type_format()

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
