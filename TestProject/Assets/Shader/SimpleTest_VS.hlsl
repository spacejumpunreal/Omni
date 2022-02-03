
struct VSOut
{
	float4 hpos : SV_POSITION;
	float4 color : TEXCOORD0;
};

VSOut VSMain(uint vid : SV_VertexID)
{
	VSOut o;
	uint x = vid % 2;
	uint y = vid / 2;
	float2 xy = float2(x, y) * 2 - 1;
	o.hpos = float4(xy * 0.5f, 0.5f, 1);
	o.color = float4(x, y, saturate(1.0f - x - y), 1);
	return o;
}
