#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include<vector>
#include<map>
#include<string>
#include<wrl.h>

class Dx12Wrapper;
class PMDRenderer;

class PMDActor
{
	friend PMDRenderer;
private:
	PMDRenderer& _renderer;
	Dx12Wrapper& _dx12;

	//�V�F�[�_�[���ɓ�������}�e���A���f�[�^
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT3 diffuse;//�f�B�t���[�Y�F
		float alpha;//�f�B�t���[�Y��
		DirectX::XMFLOAT3 specular;//�X�؃L�����F
		float specularity;//�X�؃L�����̋���(��Z�l)
		DirectX::XMFLOAT3 ambient;//�A�r�G���g�F
	};

	//����ȊO�̃}�e���A���f�[�^
	struct AdditionalMaterial
	{
		std::string texPath;
		int toonIdx;
		bool edgeFlg;
	};

	//�S�̂��܂Ƃ߂�f�[�^
	struct Material
	{
		unsigned int indicesNum;
		MaterialForHlsl material;
		AdditionalMaterial additional;
	};

	std::vector<Material> materials;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _textureResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _sphResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _spaResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _toonResources;

	Microsoft::WRL::ComPtr<ID3D12Resource> _materialBuff = nullptr;
	unsigned int _materialNum;//�}�e���A����
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _materialDescHeap = nullptr;//�}�e���A���q�[�v

	Microsoft::WRL::ComPtr<ID3D12Resource> _vertBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> _idxBuff = nullptr;
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	struct Transform {
		void* operator new(size_t size);
		DirectX::XMMATRIX world;
	};
	Transform _transform;
	Transform* _mappedTransform = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _transformBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _transformMat = nullptr;//���W�ϊ��s��(���̓��[���h�̂�)
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _transformHeap = nullptr;//���W�ϊ��q�[�v

	float _angle;//�e�X�g�pY����]

public:
	HRESULT LoadPMDFile(const char* path);//PMD�t�@�C���̃��[�h
	void CreateMaterialData();//�ǂݍ��񂾃}�e���A�������ƂɃ}�e���A���o�b�t�@���쐬
	void CreateMaterialAndTextureView();//�}�e���A�����e�N�X�`���̃r���[���쐬
	void CreateTransformView();

public:
	PMDActor(const char* filepath, PMDRenderer& renderer);
	~PMDActor();
	PMDActor* Clone();
	void Update();
	void Draw();

};