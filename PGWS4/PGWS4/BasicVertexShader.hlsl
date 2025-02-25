#include "BasicShaderHeader.hlsli"

Output BasicVS(
	float4 pos : POSITION,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint weight : WEIGHT)
{
	Output output;//ピクセルシェーダーに渡す値
	float w = (float)weight / 100.0f;
	matrix bm = bones[boneno[0]] * w + bones[boneno[1]] * (1.0f - w);
	pos = mul(bm, pos);
	pos = mul(world, pos);
	output.ray = pos.xyz - eye;
	//output.svpos = mul(proj,mul(view,mul(world, pos)));
	output.svpos = mul(proj, mul(view, pos));
	normal.w = 0;
	output.normal = mul(world, normal);
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	return output;
}