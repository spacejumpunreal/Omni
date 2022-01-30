#ifndef OMNI_BASICS_HLSL
#define OMNI_BASICS_HLSL

struct VSOut
{
	float4 hpos : SV_POSITION;
};

StructuredBuffer<float4> PositionStream;

VSOut VertexFunc(uint vid : SV_VertexID)
{
	VSOut o;
	o.hpos = PositionStream[vid];
	return o;
}

#endif//OMNI_BASICS_HLSL