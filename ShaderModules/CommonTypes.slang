module CommonTypes;

public struct Vertex
{
	public float3 position;
	float uv_x;
	public float3 normal;
	float uv_y;

    public float2 GetUV() { return float2(uv_x, uv_y); }
};

public struct VertexWithColor
{
	public float3 position;
	float uv_x;
	public float3 normal;
    float uv_y;
    public float4 color;

    float2 GetUV() { return float2(uv_x, uv_y); }
};
