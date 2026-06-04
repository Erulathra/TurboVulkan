module ShadingCommon;

import Math;
import ViewData;
#include "MathConstants.slang"

public struct FLight
{
	public float3 mColor;
	public float mIntensity;

	public float3 mPosition;
	public float mRadius;

	public float3 mDirection;
	public uint mInnerOuterAngleAndType;

	float4 _padding;
}

public enum ELightType
{
	Point,
	Spot,
	Directional,
}

public struct FInnerOuterAngle
{
	public float mInnerAngle;
	public float mOuterAngle;
	public uint mType;
}

public struct FPixelLightningInput
{
   public float3 mPosition;
   public float3 mNormal;
   public float3 mPixelToCamera;

   public float3 mAlbedo;
   public float mMetallic;
   public float mRoughness;
   public float mAO;
}

float SaturatedDot(float3 a, float3 b)
{
	return saturate(dot(a, b));
}

public FInnerOuterAngle DecodeInnerOuterAngleAndType(in uint innerAndOuterAngle)
{
    const uint maxAngleValue = (1 << 15) - 1;

	const uint innerAngle = innerAndOuterAngle >> 17;
	const uint outerAngle = (innerAndOuterAngle & (maxAngleValue << 2)) >> 2;
	const uint type = innerAndOuterAngle & 0x3;

    FInnerOuterAngle outResult;
	outResult.mInnerAngle =  float(innerAngle) / float(maxAngleValue) * M_PI;
	outResult.mOuterAngle =  float(outerAngle) / float(maxAngleValue) * M_PI;
	outResult.mType = type;

	return outResult;
}

public ELightType GetLightType(in FLight light)
{
	return (ELightType)(light.mInnerOuterAngleAndType & 0x3);
}

public float CalculateLightAttentuation(float distance, float lightIntensity, float lightRange)
{
    // Godot's attenuation formula
    float nd = distance * rcp(lightRange);
    nd *= nd;
    nd *= nd;
    nd = max(1.f - nd, 0.f);
    nd *= nd;
    return nd * pow(max(distance, TURBO_KINDA_SMALL_NUMBER), /** decay */ -2.f);
}

public float3 CalculateFresnel(float cosTheta, float3 F0)
{
    // Fresnel-Schlick (LearnOpenGL)
    return F0 + (1.f - F0) * pow(saturate(1.f - cosTheta), 5.f);
}

public float CalculateNormalDistribution(float3 normal, float3 halfVector, float alpha)
{
    // Trowbridge-Reitz (LearnOpenGL)
    const float alphaSquared = alpha * alpha;
    const float cosTheta = SaturatedDot(normal, halfVector);
    const float cosTheteSquared = cosTheta * cosTheta;

    float denominator = cosTheteSquared * (alphaSquared - 1.f) + 1.f;
    denominator = M_PI * denominator * denominator;

    return alphaSquared * rcp(denominator);
}

public float CalculateShlickGGX(float3 normal, float3 rayDirection, float alpha)
{
    // Shlick GGX (LearnOpenGL)
    const float kDirect = (alpha + 1.f) * (alpha + 1.f) * rcp(8.f);
    const float cosTheta = SaturatedDot(normal, rayDirection);

    return cosTheta * rcp(cosTheta * (1.f - kDirect) + kDirect);
}

public float CalculateGeometryShading(float3 normal, float3 lightToCamera, float3 lightToPixel, float alpha)
{
    const float geometryObstruction = CalculateShlickGGX(normal, lightToCamera, alpha);
    const float geometryShadowing = CalculateShlickGGX(normal, lightToPixel, alpha);

    return geometryObstruction * geometryShadowing;
}

public float3 CalculatePixelRadiance(in FLight light, in FPixelLightningInput input)
{
    const float3 pixelToLightVector = light.mPosition - input.mPosition;
    const float lightDistance = length(pixelToLightVector);

    float attenuation = 1.f;
    if (GetLightType(light) != ELightType::Directional)
    {
        attenuation = CalculateLightAttentuation(lightDistance, light.mIntensity, light.mRadius);
    }

    const float3 lightToPixel = pixelToLightVector / lightDistance;
    const float3 halfVector = normalize(lightToPixel + input.mPixelToCamera);

    // F0 hack, we assume that all dielectric surfaces have a F0 of 0.04
    const float3 f0 = lerp(0.04f, input.mAlbedo, input.mMetallic);
    const float3 fresnel = CalculateFresnel(dot(halfVector, input.mPixelToCamera), f0);

    const float alpha = input.mRoughness * input.mRoughness;
    const float normalDistribution = CalculateNormalDistribution(input.mNormal, halfVector, alpha);
    const float geometry = CalculateGeometryShading(input.mNormal, input.mPixelToCamera, lightToPixel, alpha);

    float denominator = 4.f * SaturatedDot(input.mNormal, lightToPixel) * SaturatedDot(input.mNormal, input.mPixelToCamera);
    denominator = max(denominator, TURBO_SMALL_NUMBER);
    const float3 brdf = fresnel * (normalDistribution * geometry * rcp(denominator));

    const float3 kReflected = fresnel;
    const float3 kDiffuse = (1.f - kReflected) * (1.f - input.mMetallic);

    const float cosTheta = SaturatedDot(input.mNormal, lightToPixel);
    return (kDiffuse * input.mAlbedo / M_PI + brdf) * light.mColor * light.mIntensity * attenuation * cosTheta;
}
