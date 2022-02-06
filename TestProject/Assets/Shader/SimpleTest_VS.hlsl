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
ByteAddressBuffer VertexStream0Buffer : register(t0, space0);

VSOut VSMain(uint vid : SV_VertexID)
{
	VSOut o;
#if false
	uint x = vid % 2;
	uint y = vid / 2;
	float4 worldPos = float4(x, y, 0, 1);
#else
	float4 worldPos;
	uint vs0size = sizeof(VertexStream0);
	worldPos.xyz = VertexStream0Buffer.Load<VertexStream0>(vid * vs0size).Position;
	worldPos.w = 1;
#endif
	o.hpos = mul(CB0.VPMatrix, worldPos);
	o.color = float4(worldPos.x, worldPos.y, saturate(1.0f - worldPos.x - worldPos.y), 1);
	return o;
}
