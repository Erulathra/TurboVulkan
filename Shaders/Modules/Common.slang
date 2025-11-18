[vk_binding(0, 0)]
public Texture2D<float3> texturePool[];

[vk_binding(1, 0)]
public Texture2D<float3> rwTexturePool[];

[vk_binding(2, 0)]
public SamplerState samplerPool[];
