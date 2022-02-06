struct VertexStream0
{
	float3 Position;
};


struct VSOut
{
	float4 hpos : SV_POSITION;
	float4 color : TEXCOORD0;
};

struct MyConstants
{
	float4x4 VPMatrix;
};

ConstantBuffer<MyConstants> CB0 : register(b0, space0);

VSOut VSMain(uint vid : SV_VertexID)
{
	VSOut o;
	uint x = vid % 2;
	uint y = vid / 2;
	float4 worldPos = float4(x, y, 0, 1);
	o.hpos = mul(CB0.VPMatrix, worldPos);
	o.color = float4(x, y, saturate(1.0f - x - y), 1);
	return o;
}
