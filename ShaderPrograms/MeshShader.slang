import CommonTypes;

[[vk::push_constant]]
cbuffer PushConstants
{
    float4x4 world2View;

    float3* positions;
    float3* normals;
    float4* colors;
};

struct VSOut
{
    float4 position : SV_Position;
    float3 worldNormal;
    float3 color;
};

[shader("vertex")]
VSOut vsMain(in uint vertexId : SV_VertexID)
{
    VSOut result;
    float3 position = positions[vertexId];
    float3 normal = normals[vertexId];
    float4 color = colors[vertexId];

    result.position = mul(world2View, float4(position, 1.f));
    result.worldNormal = normals[vertexId];
    result.color = color.xyz;

    return result;
}

[shader("pixel")]
float4 psMain(in VSOut vsOut) : SV_Target0
{
    const float3 sunDirection = normalize(float3(-0.5f, -1.f, -0.5f));
    const float ambientLight = 0.2f;
    float lightStrenght = saturate(saturate(dot(vsOut.worldNormal, -sunDirection)) + ambientLight);

    const float3 outColor = vsOut.color * lightStrenght;
    return float4(outColor, 1.f);
}
