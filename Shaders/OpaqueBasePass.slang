#include "Modules/Common.slang"

import Modules.BasePassCommon;
import Modules.MeshRendering;
import Modules.Math;
import Modules.ShadingCommon;

[[vk::push_constant]]
FIndirectPushConstants pc;

struct FMaterialData
{
    uint mBaseColorSampler;
    uint mORMSampler;
    uint mNormalSampler;
}

struct FInstanceData
{
    uint mBaseColorTexture;
    uint mORMTexture;
    uint mNormalTexture;

    float mNormalScale;
    float4 mBaseColorFactor;
    float mMetalicFactor;
    float mRoughnessFactor;
}

struct VSOut
{
    float4 mPosition : SV_Position;

    float3 mWorldPosition;
    float3 mWorldNormal;
    float2 mUV;

	uint mDrawId;
}

struct PSOut
{
    float3 mColor : SV_Target0;
}

[shader("vertex")]
void vsMain(
    in uint indexId : SV_VertexID,
	in uint drawId : SV_StartInstanceLocation,
    out VSOut vsOut
)
{
	const FIndirectDrawData data = pc.mDrawData[drawId];
    const Ptr<FMeshData> mesh = data.mMeshData;

	const uint vertexId = asuint(mesh.mIndexBuffer[indexId]);
	const float4 modelPosition = float4(mesh.mPositionBuffer[vertexId], 1.f);

    vsOut.mPosition = mul(modelPosition, data.mModelToProj);

    vsOut.mWorldPosition = mul(modelPosition, data.mModelToWorld).xyz;
	vsOut.mWorldNormal = mul(mesh.mNormalBuffer[vertexId], data.mNormalModelToWorld);
    vsOut.mUV = mesh.mUVBuffer[vertexId];

	vsOut.mDrawId = drawId;
}

[shader("pixel")]
void psMain(
    in VSOut vsOut,
    out PSOut psOut
)
{
	const FIndirectDrawData data = pc.mDrawData[vsOut.mDrawId];

    const Ptr<FInstanceData> instance = Ptr<FInstanceData>(data.mMaterialInstance);
    const Ptr<FMaterialData> material = Ptr<FMaterialData>(data.mMaterialData);

    // Fetch textures and samplers
    Texture2D<float3> albedoTexture = texturePool[instance.mBaseColorTexture];
    SamplerState albedoSampler = samplerPool[material.mBaseColorSampler];
    Texture2D<float3> normalTexture = texturePool[instance.mNormalTexture];
    SamplerState normalSampler = samplerPool[material.mNormalSampler];
    Texture2D<float3> ormTexture = texturePool[instance.mORMTexture];
    SamplerState ormSampler = samplerPool[material.mORMSampler];

    // TODO: Normal mapping

    Ptr<FViewData> viewData = pc.mViewData;

    // Fill pixel data
    FPixelLightningInput pixelInput;
    pixelInput.mPosition = vsOut.mWorldPosition;
    pixelInput.mNormal = vsOut.mWorldNormal;
    pixelInput.mPixelToCamera = normalize(viewData.mViewPosition - vsOut.mWorldPosition);

    pixelInput.mAlbedo = albedoTexture.Sample(albedoSampler, vsOut.mUV);
    pixelInput.mAlbedo *= instance.mBaseColorFactor.rgb;

    const float3 ORM = ormTexture.Sample(ormSampler, vsOut.mUV);
    pixelInput.mMetallic = ORM.r * instance.mMetalicFactor;
    pixelInput.mRoughness = ORM.g * instance.mRoughnessFactor;
    pixelInput.mAO = ORM.b;

    Ptr<FSceneData> scene = pc.mSceneData;
    Ptr<FLight> lights = pc.mLightData;

    float3 irradiance = 0.f;

    // Calculate irradiance
    for (int lightIndex = 0; lightIndex < scene.mNumLights; lightIndex++)
    {
        // For now only support point lights
        FLight light = lights[lightIndex];
        if (GetLightType(light) == ELightType::Point)
        {
            irradiance += CalculatePixelRadiance(light, pixelInput);
        }
    }

    // Add ambient light
    irradiance += 0.03f * pixelInput.mAlbedo;

    psOut.mColor = irradiance;
}
