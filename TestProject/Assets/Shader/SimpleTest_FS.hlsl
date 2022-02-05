
struct VSOut
{
	float4 hpos : SV_POSITION;
	float4 color : TEXCOORD0;
};



float4 FSMain(VSOut vsout) : SV_TARGET0
{
	return vsout.color;
}
