#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	return float4(tex.Sample(smp,input.uv));
}

//-----------�`�������W�ۑ�(��4��) ���������̌`-----------//

//float4 BasicPS(float4 pos:POSITION) : SV_TARGET
//{
//	return float4(1.0f,(float2(1,0) + pos.xy) * 0.5f,1);
//}

//-------------------------------------------------------///


