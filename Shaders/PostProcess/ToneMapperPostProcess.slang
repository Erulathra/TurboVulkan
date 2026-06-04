#include "Modules/Common.slang"

struct FUniformParams
{
    float mExposure;
    float mSaturation;

    float3 mOffset;
    float3 mSlope;
    float3 mPower;
};

struct FPushConstants
{
    uint mSceneColor;
    uint mOutput;
    uint2 mTextureSize;

    Ptr<FUniformParams> mUniforms;
};

[[vk::push_constant()]]
FPushConstants pc;

float3 AGXContrastApproximation(float3 x)
{
    const float3 x2 = x * x;
    const float3 x4 = x2 * x2;

    return
       + 15.5f * x4 * x2
       - 40.14f * x4 * x
       + 31.96f * x4
       - 6.868f * x2 * x
       + 0.4298f * x2
       + 0.1191f * x
       - 0.00232f;
}

float3 AGX(float3 color)
{
    const float3x3 agxMatrix = float3x3(
        0.842479062253094f, 0.0423282422610123f, 0.0423756549057051f,
        0.0784335999999992f, 0.878468636469772f, 0.0784336f,
        0.0792237451477643f, 0.0791661274605434f, 0.879142973793104f
    );

    // Original values
    const float minEv = -12.47393f;
    const float maxEv = 4.026069f;

    // Input transform
    color = mul(agxMatrix, color);

    // Log2 encoding
    color = clamp(log2(color), minEv, maxEv);
    color = (color - minEv) / (maxEv - minEv);

    color = AGXContrastApproximation(color);

    return color;
}

float3 AGXEotf(float3 color)
{
    const float3x3 agxMatrixInv = float3x3(
        1.19687900512017f, -0.0528968517574562f, -0.0529716355144438f,
        -0.0980208811401368f, 1.15190312990417f, -0.0980434501171241f,
        -0.0990297440797205f, -0.0989611768448433f, 1.15107367264116f
    );

    color = mul(agxMatrixInv, color);

    // Gamma correction
    color = pow(color, 2.2f);

    return color;
}

float3 AGXLook(float3 color)
{
    color = pow(color * pc.mUniforms.mSlope + pc.mUniforms.mOffset, pc.mUniforms.mPower);
    const float3 luminanceWeights = float3(0.2126f, 0.7152f, 0.0722f);
    const float luma = dot(color, luminanceWeights);

    return luma + pc.mUniforms.mSaturation * (color - luma);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    if (any(threadId.xy >= pc.mTextureSize))
    {
        return;
    }

    Texture2D<float3> sceneColor = texturePool[pc.mSceneColor];
    RWTexture2D<float4> output = rwTexturePool[pc.mOutput];

    const float3 hdrColor = sceneColor[threadId.xy];

    float3 postTonemap = hdrColor * pc.mUniforms.mExposure;
    postTonemap = AGX(postTonemap);
    postTonemap = AGXLook(postTonemap);
    postTonemap = AGXEotf(postTonemap);

    output[threadId.xy] = float4(postTonemap, 1.f);
}
