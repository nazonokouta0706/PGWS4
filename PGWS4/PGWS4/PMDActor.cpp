#include "PMDActor.h"
#include "PMDRenderer.h"
#include "Dx12Wrapper.h"
#include <d3dx12.h>

using namespace DirectX;
using namespace Microsoft::WRL;

static  inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

static std::string GetTexturePathFromModelAndTexPath(
	const std::string& modelPath,
	const char* texPath)
{
	int pathIndex1 = static_cast<int>(modelPath.rfind('/'));
	int pathIndex2 = static_cast<int>(modelPath.rfind('\\'));
	int pathIndex = max(pathIndex1, pathIndex2);
	std::string folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}

static std::pair<std::string, std::string>SplitFileName(
	const std::string& path, const char splitter = '*')
{
	int idx = static_cast<int>(path.find(splitter));
	std::pair<std::string, std::string>ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(
		idx + 1, path.length() - idx - 1);
	return ret;
}

static std::string GetExtension(const std::string& path) {
	int idx = static_cast<int>(path.rfind('.'));
	return path.substr(
		idx + 1, path.length() - idx - 1);
}

//  Model/�������J.pmd �����~�Nmetal.pmd �����~�N.pmd
HRESULT PMDActor::LoadPMDFile(const char* path) {

	//PMD �w�b�_�[�\����
	struct PMDHeader
	{
		float version;//00 00 80 3F == 1.00
		char model_name[20];//���f����
		char comment[256];// ���f���R�����g
	};

	char signature[3] = {};//�V�O�l�`��
	PMDHeader pmdheader = {};

	std::string strModelPath = path;
	FILE* fp;
	fopen_s(&fp, strModelPath.c_str(), "rb");

	if (fp == nullptr) {
		assert(0);	return ERROR_FILE_NOT_FOUND;
	}

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;//���_�����擾
	fread(&vertNum, sizeof(vertNum), 1, fp);


#pragma pack(1)
	//PMD�}�e���A���\����
	struct PMDMaterial
	{
		XMFLOAT3 diffuse;//�f�B�t���[�Y�F
		float alpha;//�f�B�t���[�Y��
		float specularity;//�X�؃L�����̋���(��Z�l)
		XMFLOAT3 specular;//�X�؃L�����F
		XMFLOAT3 ambient;//�A�r�G���g�F
		unsigned char toonIdx;//�g�D�[���ԍ�(��q)
		unsigned char edgeFlg;

		unsigned int indicesNum;
								
		char texFilePath[20];  
	};
#pragma pack()

	constexpr unsigned int pmdvertex_size = 38;//���_1������̃T�C�Y
#pragma pack(push,1)
	struct PMD_VERTEX
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		uint16_t bone_no[2];
		uint8_t weight;
		uint8_t EdgeFlag;
		uint16_t dummy;
	};
#pragma pack(pop)

	std::vector<PMD_VERTEX> vertices(vertNum);//�o�b�t�@�[�̊m��
	for (unsigned int i = 0; i < vertNum; i++) {
		fread(&vertices[i], pmdvertex_size, 1, fp);
	}

	auto heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resdesc = CD3DX12_RESOURCE_DESC::Buffer(vertices.size() * sizeof(PMD_VERTEX));

	ComPtr<ID3D12Device> dev = _dx12.Device();

	ThrowIfFailed(_dx12.Device()->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,//�T�C�Y�ύX
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_vertBuff.ReleaseAndGetAddressOf())));

	PMD_VERTEX* vertMap = nullptr;
	ThrowIfFailed(_vertBuff->Map(0, nullptr, (void**)&vertMap));
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	_vertBuff->Unmap(0, nullptr);

	_vbView.BufferLocation = _vertBuff->GetGPUVirtualAddress();//�o�b�t�@�[�̉��z�A�h���X
	_vbView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(PMD_VERTEX));//�S�o�C�g��
	_vbView.StrideInBytes = sizeof(vertices[0]);//�P���_������̃o�C�g��

	unsigned int indicesNum;//�C���f�b�N�X��
	fread(&indicesNum, sizeof(indicesNum), 1, fp);

	std::vector<unsigned short> indices;
	indices.resize(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	resdesc.Width = indices.size() * sizeof(indices[0]);
	ThrowIfFailed(dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_idxBuff.ReleaseAndGetAddressOf())));

	//������o�b�t�@�[�ɃC���f�b�N�X�f�[�^�����R�s�[
	unsigned short* mapppedIdx = nullptr;
	_idxBuff->Map(0, nullptr, (void**)&mapppedIdx);
	std::copy(std::begin(indices), std::end(indices), mapppedIdx);
	_idxBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�[�r���[���쐬
	_ibView.BufferLocation = _idxBuff->GetGPUVirtualAddress();
	_ibView.Format = DXGI_FORMAT_R16_UINT;
	_ibView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(indices[0]));

	fread(&_materialNum, sizeof(_materialNum), 1, fp);

	_textureResources.resize(_materialNum);
	_sphResources.resize(_materialNum);
	_spaResources.resize(_materialNum);
	_toonResources.resize(_materialNum);

	std::vector<PMDMaterial> pmdMaterials(_materialNum);

	fread(
		pmdMaterials.data(),
		pmdMaterials.size() * sizeof(PMDMaterial),
		1,
		fp
	);

	materials.resize(pmdMaterials.size());
	//�R�s�[
	for (int i = 0; i < pmdMaterials.size(); ++i)
	{
		materials[i].indicesNum = pmdMaterials[i].indicesNum;
		materials[i].material.diffuse = pmdMaterials[i].diffuse;
		materials[i].material.alpha = pmdMaterials[i].alpha;
		materials[i].material.specular = pmdMaterials[i].specular;
		materials[i].material.specularity = pmdMaterials[i].specularity;
		materials[i].material.ambient = pmdMaterials[i].ambient;
		materials[i].additional.toonIdx = pmdMaterials[i].toonIdx;
	}

	for (int i = 0; i < pmdMaterials.size(); ++i)
	{

		//�g�D�[�����\�[�X�̓ǂݍ��� toon2(�`�������W�p���Z���߂̃g�D�[��)
		std::string toonFilePath = "toon/";
		char toonFileName[16];
		sprintf_s(toonFileName, "toon%02d.bmp", pmdMaterials[i].toonIdx + 1);
		toonFilePath += toonFileName;
		_toonResources[i] = _dx12.GetTextureByPath(toonFilePath.c_str());

		if (strlen(pmdMaterials[i].texFilePath) == 0) { continue; }

		std::string texFileName = pmdMaterials[i].texFilePath;
		std::string sphFileName = "";
		std::string spaFileName = "";

		if (std::count(texFileName.begin(), texFileName.end(), '*') > 0)
		{//�X�v���b�^������

			auto namepair = SplitFileName(texFileName);

			if (GetExtension(namepair.first) == "sph")
			{
				texFileName = namepair.second;
				sphFileName = namepair.first;
			}
			else if (GetExtension(namepair.first) == "spa")
			{
				texFileName = namepair.second;
				spaFileName = namepair.first;
			}
			else
			{
				texFileName = namepair.first;//���̊g���q�ł��Ƃɂ����ŏ��̕������Ă���
				if (GetExtension(namepair.second) == "sph")
				{
					sphFileName = namepair.second;
				}
				else if (GetExtension(namepair.second) == "spa")
				{
					spaFileName = namepair.second;
				}
			}
		}
		else {
			std::string ext = GetExtension(pmdMaterials[i].texFilePath);
			if (ext == "sph")
			{
				sphFileName = pmdMaterials[i].texFilePath;
				texFileName = "";
			}
			else if (ext == "spa")
			{
				spaFileName = pmdMaterials[i].texFilePath;
				texFileName = "";
			}
		}

		//���f���ƃe�N�X�`���p�X����A�v���P�[�V��������̃e�N�X�`���p�X�𓾂�B
		auto texFilePath = GetTexturePathFromModelAndTexPath(
			strModelPath,texFileName.c_str());
		_textureResources[i] = _dx12.GetTextureByPath(texFilePath.c_str());

		if (sphFileName != "") {
			auto sphFilePath = GetTexturePathFromModelAndTexPath(
				strModelPath,sphFileName.c_str());
			_sphResources[i] =  _dx12.GetTextureByPath(sphFilePath.c_str());
		}
		if (spaFileName != "") {
			auto spaFilePath = GetTexturePathFromModelAndTexPath(
				strModelPath,spaFileName.c_str());
			_spaResources[i] =  _dx12.GetTextureByPath(spaFilePath.c_str());
		}
	}

	//���̐��̓ǂݍ���
	unsigned short boneNum = 0;
	fread(&boneNum, sizeof(boneNum), 1, fp);

#pragma pack(1)
	//�ǂݍ��ݗp�{�[���\����
	struct PMDBone {
		char boneName[20];			//�{�[����
		unsigned short parentNo;	//�e�{�[���ԍ�
		unsigned short nextNo;		//��[�̃{�[���ԍ�
		unsigned char type;			//�{�[�����
		unsigned short ikboneNo;	//IK�{�[���ԍ�
		XMFLOAT3 pos;				//�{�[���̊�_���W
	};
#pragma pack()

	std::vector<PMDBone> pmdBones(boneNum);
	fread(pmdBones.data(), sizeof(PMDBone), boneNum, fp);


	fclose(fp);

	// �C���f�b�N�X�Ɩ��O�̑Ή��֌W�\�z�̂��߂Ɍ�Ŏg��
	std::vector<std::string> boneNames(pmdBones.size());

	// �{�[���m�[�h�}�b�v�����
	for (int idx = 0; idx < pmdBones.size(); ++idx)
	{
		PMDBone& pb = pmdBones[idx];
		boneNames[idx] = pb.boneName;
		BoneNode& node = _boneNodeTable[pb.boneName];
		node.boneIdx = idx;
		node.startPos = pb.pos;
	}

	// �e�q�֌W���\�z����
	for (PMDBone& pb : pmdBones)
	{
		// �e�C���f�b�N�X���`�F�b�N(���蓾�Ȃ��ԍ��Ȃ��΂�)
		if (pmdBones.size() <= pb.parentNo)
		{
			continue;
		}
		std::string& parentName = boneNames[pb.parentNo];
		_boneNodeTable[parentName].children.
			emplace_back(&_boneNodeTable[pb.boneName]);
	}

	// �{�[�������ׂď���������B
	_boneMatrices.resize(pmdBones.size());
	std::fill(_boneMatrices.begin(), _boneMatrices.end(), XMMatrixIdentity());

	return S_OK;
}

void PMDActor::CreateMaterialData() {
	//�}�e���A���o�b�t�@�[�̍쐬
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	const D3D12_HEAP_PROPERTIES heapPropMat =
		CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const D3D12_RESOURCE_DESC resDescMat = CD3DX12_RESOURCE_DESC::Buffer(
		materialBuffSize * _materialNum);
	auto _dev = _dx12.Device();
	ThrowIfFailed(_dev->CreateCommittedResource(
		&heapPropMat,
		D3D12_HEAP_FLAG_NONE,
		&resDescMat,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_materialBuff.ReleaseAndGetAddressOf())
	));

	//�}�b�v�}�e���A���ɃR�s�[
	char* mapMaterial = nullptr;
	ThrowIfFailed(_materialBuff->Map(0, nullptr, (void**)&mapMaterial));
	for (auto& m : materials) {
		*((MaterialForHlsl*)mapMaterial) = m.material;
		mapMaterial += materialBuffSize;
	}
	_materialBuff->Unmap(0, nullptr);
}

void PMDActor::CreateMaterialAndTextureView() {

	ID3D12Device* _dev = _dx12.Device().Get();

	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;
	materialDescHeapDesc.NumDescriptors = _materialNum * 5; // �}�e���A�������i�萔1�A�e�N�X�`��4�j
	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ThrowIfFailed(_dev->CreateDescriptorHeap(
		&materialDescHeapDesc, IID_PPV_ARGS(_materialDescHeap.ReleaseAndGetAddressOf())));

	UINT materialBuffSize = (sizeof(MaterialForHlsl) + 0xff) & ~0xff;
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = _materialBuff->GetGPUVirtualAddress(); // �o�b�t�@�[�A�h���X
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize); // �}�e���A����256 �A���C�����g�T�C�Y

	//�ʏ�e�N�X�`���r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH(_materialDescHeap->GetCPUDescriptorHandleForHeapStart());
	UINT incSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (UINT i = 0; i < _materialNum; ++i)
	{
		_dev->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += static_cast<UINT>(materialBuffSize);

		if (_textureResources[i] == nullptr) {
			srvDesc.Format = _renderer._whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_renderer._whiteTex.Get(),&srvDesc,matDescHeapH);
		}
		else
		{
			srvDesc.Format = _textureResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_textureResources[i].Get(),&srvDesc,matDescHeapH);
		}

		matDescHeapH.ptr += incSize;

		if (_sphResources[i] == nullptr) {
			srvDesc.Format = _renderer._whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_renderer._whiteTex.Get(),&srvDesc,matDescHeapH);
		}
		else
		{
			srvDesc.Format = _sphResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_sphResources[i].Get(),&srvDesc,matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (_spaResources[i] == nullptr) {
			srvDesc.Format = _renderer._blackTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_renderer._blackTex.Get(),&srvDesc,matDescHeapH);
		}
		else
		{
			srvDesc.Format = _spaResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_spaResources[i].Get(),&srvDesc,matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (_toonResources[i] == nullptr) {
			srvDesc.Format = _renderer._gradTex->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_renderer._gradTex.Get(),&srvDesc,matDescHeapH);
		}
		else
		{
			srvDesc.Format = _toonResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(
				_toonResources[i].Get(),&srvDesc,matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

	}

}

void PMDActor::CreateTransformView()
{
	unsigned int buffSize = static_cast<UINT>(sizeof(XMMATRIX) * (1 + _boneMatrices.size()));
	buffSize = (buffSize + 0xff) & ~0xff;
	CD3DX12_HEAP_PROPERTIES heapProp(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	ThrowIfFailed(_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_transformBuff.ReleaseAndGetAddressOf())
	));

	ThrowIfFailed(_transformBuff->Map(0, nullptr, (void**)&_mappedMatrices));

	_mappedMatrices[0] = _transform.world;

	std::copy(_boneMatrices.begin(), _boneMatrices.end(), _mappedMatrices + 1);

	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(_dx12.Device()->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(_transformHeap.ReleaseAndGetAddressOf())));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = buffSize;
	_dx12.Device()->CreateConstantBufferView(&cbvDesc, _transformHeap->GetCPUDescriptorHandleForHeapStart());
}

void* PMDActor::Transform::operator new(size_t size) {
	return _aligned_malloc(size, 16);
}

PMDActor::PMDActor(const char* filepath, PMDRenderer& renderer) :
	_renderer(renderer),
	_dx12(renderer._dx12),
	_angle(0.0f)
{
	ThrowIfFailed(LoadPMDFile(filepath));

	CreateMaterialData();
	CreateMaterialAndTextureView();

	_transform.world = XMMatrixIdentity();
	CreateTransformView();
}

PMDActor::~PMDActor()
{
}

void PMDActor::LoadVMDFile(const char* filepath, const char* name)
{
	FILE* fp;
	fopen_s(&fp, filepath, "rb");
	fseek(fp, 50, SEEK_SET); 

	unsigned int keyframeNum = 0;
	fread(&keyframeNum, sizeof(keyframeNum), 1, fp);

	struct VMDKeyFrame
	{
		char boneName[15];		  // �{�[����
		unsigned int frameNo;	  // �t���[���ԍ�
		XMFLOAT3 location;		  // �ʒu
		XMFLOAT4 quaternion;	  // ��]
		unsigned char bezier[64]; // �x�W�F�⊮�p�����[�^
	};

	std::vector<VMDKeyFrame> keyframes(keyframeNum);
	for (VMDKeyFrame& keyframe : keyframes)
	{
		fread(keyframe.boneName, sizeof(keyframe.boneName), 1, fp); // �{�[����
		fread(&keyframe.frameNo, sizeof(keyframe.frameNo) +			// �t���[���ԍ�
			sizeof(keyframe.location) +								// �ʒu
			sizeof(keyframe.quaternion) +							// �N�I�[�^�j�I��
			sizeof(keyframe.bezier),								// ��ԃx�W�F�f�[�^
			1, fp);
	}

	fclose(fp);

	// VMD�̃L�[�t���[���f�[�^����A���ۂɎg�p����L�[�t���[���e�[�u���֕ϊ�
	for (VMDKeyFrame& f : keyframes)
	{
		_motiondata[f.boneName].emplace_back(
			KeyFrame(f.frameNo,
				XMLoadFloat4(&f.quaternion)));
	}

	for (auto& bonemotion : _motiondata)
	{
		BoneNode& node = _boneNodeTable[bonemotion.first];
		DirectX::XMFLOAT3& pos = node.startPos;
		DirectX::XMMATRIX mat =
			XMMatrixTranslation(-pos.x, -pos.y, -pos.z) *
			XMMatrixRotationQuaternion(bonemotion.second[0].quaternion) *
			XMMatrixTranslation(pos.x, pos.y, pos.z);
		_boneMatrices[node.boneIdx] = mat;
	}

	RecursiveMatrixMultipy(_boneNodeTable["�Z���^�["], XMMatrixIdentity());
	copy(_boneMatrices.begin(), _boneMatrices.end(), _mappedMatrices + 1);
}


void PMDActor::RecursiveMatrixMultipy(BoneNode& node, const DirectX::XMMATRIX& mat)
{
	_boneMatrices[node.boneIdx] = mat;

	for (BoneNode* cnode : node.children) {
		RecursiveMatrixMultipy(*cnode, _boneMatrices[cnode->boneIdx] * mat);
	}
}


void PMDActor::Update()
{
	/*_angle += 0.03f;
	_mappedMatrices[0] = XMMatrixRotationY(_angle);*/

	////�s����N���A(���ĂȂ��ƑO�t���[���̃|�[�Y���d�ˊ|������ă��f��������)
	//std::fill(_boneMatrices.begin(), _boneMatrices.end(), XMMatrixIdentity());

	//// ���r��90���Ȃ���
	//BoneNode& armNode = _boneNodeTable["���r"];
	//DirectX::XMFLOAT3& armPos = armNode.startPos;
	//DirectX::XMMATRIX armMat =
	//	XMMatrixTranslation(-armPos.x, -armPos.y, -armPos.z)
	//	* XMMatrixRotationZ(XM_PIDIV2)
	//	* XMMatrixTranslation(armPos.x, armPos.y, armPos.z);

	//// ���Ђ���-90���Ȃ���
	//BoneNode& elbowNode = _boneNodeTable["���Ђ�"];
	//DirectX::XMFLOAT3& elbowPos = elbowNode.startPos;
	//DirectX::XMMATRIX elbowMat =
	//	XMMatrixTranslation(-elbowPos.x, -elbowPos.y, -elbowPos.z)
	//	* XMMatrixRotationZ(-XM_PIDIV2)
	//	* XMMatrixTranslation(elbowPos.x, elbowPos.y, elbowPos.z);

	//_boneMatrices[armNode.boneIdx] = armMat;
	//_boneMatrices[elbowNode.boneIdx] = elbowMat;

	//// ������ċA�������Đe�̉e����`���������̂��ɃR�s�[
	//RecursiveMatrixMultipy(_boneNodeTable["�Z���^�["], XMMatrixIdentity());
	//copy(_boneMatrices.begin(), _boneMatrices.end(), _mappedMatrices + 1);
}

void PMDActor::Draw()
{
	ID3D12GraphicsCommandList* cmdList = _dx12.CommandList().Get();

	ID3D12DescriptorHeap* transheaps[] = { _transformHeap.Get() };
	cmdList->SetDescriptorHeaps(1, transheaps);
	cmdList->SetGraphicsRootDescriptorTable(1, 
		_transformHeap->GetGPUDescriptorHandleForHeapStart());

	cmdList->IASetVertexBuffers(0, 1, &_vbView);
	cmdList->IASetIndexBuffer(&_ibView);

	ID3D12DescriptorHeap* mdh[] = { _materialDescHeap.Get() };
	cmdList->SetDescriptorHeaps(1, mdh);

	D3D12_GPU_DESCRIPTOR_HANDLE materialH = 
		_materialDescHeap->GetGPUDescriptorHandleForHeapStart();

	unsigned int idxOffset = 0;
	UINT cbvsrvIncSize = _dx12.Device()->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;

	for (Material& m : materials)
	{
		cmdList->SetGraphicsRootDescriptorTable(2, materialH);
		cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
		materialH.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum;
	}
}



