#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	//���̌������x�N�g��(���s�����x�N�g��)
	float3 light = normalize(float3(1,-1,1));

	//���C�g�J���[(1,1,1�Ő^����)
	float3 lightColor = float3(1.0, 1.0, 1.0);

	//�f�B�t���[�Y�v�Z
	float3 normal = normalize(input.normal.xyz);
	float diffuseB = saturate(dot(-light, normal));
	
	//���̔��˃x�N�g��
	float3 refLight = reflect(light, normal);
	float specularB = pow(saturate(dot(refLight, -normalize(input.ray))), specular.a);
	//�X�t�B�A�}�b�v�puv
	float2 sphereMapUV = normalize(input.vnormal).xy * float2(0.5, -0.5) + 0.5;

	//�e�N�X�`���J���[
	float4 texColor = tex.Sample(smp, input.uv);
	return float4(lightColor *//���C�g�J���[
		(texColor.rgb//�e�N�X�`���J���[
			* sph.Sample(smp, sphereMapUV).rgb//�X�t�B�A�}�b�v(��Z)
			* (ambient + diffuseB * diffuse.rgb)//�����{�f�B�t���[�Y�F
			+ spa.Sample(smp, sphereMapUV).rgb//�X�t�B�A�}�b�v(���Z)
			+ specularB * specular.rgb)//�X�؃L����
		, diffuse.a);//�A���t�@
}
