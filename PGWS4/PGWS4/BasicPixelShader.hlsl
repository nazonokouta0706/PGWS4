//float4 BasicPS() : SV_TARGET
//{
//	return float4(1.0f, 1.0f, 1.0f, 1.0f);
//}

//-------------------�����Ă݂悤---------------------------//
//float4 BasicPS(float4 pos:POSITION) : SV_TARGET
//{
//	return float4((float2(0,1) + pos.xy)*0.5f,1,1);
//}
//----------------------------------------------------------//

//-----------�`�������W�ۑ� ���������̌`-----------//

float4 BasicPS(float4 pos:POSITION) : SV_TARGET
{
	return float4(1.0f,(float2(1,0) + pos.xy) * 0.5f,1);
}

//-------------------------------------------------------///


