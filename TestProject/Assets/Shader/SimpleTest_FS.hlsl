
struct VSOut
{
	float4 hpos : SV_POSITION;
	float4 color : TEXCOORD0;
};

struct MyConstants
{
	float4 Color;
};

ConstantBuffer<MyConstants> CB0 : register(b0, space0);

float4 FSMain(VSOut vsout) : SV_TARGET0
{
	//return vsout.color;
	return CB0.Color;
}
