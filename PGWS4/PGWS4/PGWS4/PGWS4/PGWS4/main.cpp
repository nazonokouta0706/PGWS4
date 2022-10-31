#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace std;
using namespace DirectX;

XMFLOAT3 vertices[3]; // 3 ���_

void DebugOutFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);

}

#ifdef _DEBUG
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (!SUCCEEDED(result))return;

	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}
#endif // DEBUG

// �A���C�����g�ɂ��낦���T�C�Y��Ԃ�
// @param size ���̃T�C�Y @param alignment �A���C�����g�T�C�Y @return �A���C�����g�����낦���T�C�Y
size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
}


#ifdef _DEBUG

int main()
{
#else
int WINAPI WinMain(HINSTANCE < HINSTANCE, LPSTR, int)
{
#endif
	/*DebugOutFormatString("Show window test.");
	getchar();*/
	const unsigned int window_width = 1280;
	const unsigned int window_height = 720;

	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = _T("DX12Sample");
	w.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&w);

	RECT wrc = { 0,0,window_width,window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(w.lpszClassName,
		_T("DX12Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);
	/*HRESULT D3D12CreateDevice(
		IUnknown * pAdapter,
		D3D_FEATURE_LEVEL MinimumFeatureLevel,
		REFIID riid,
		void** ppDevice
	);*/
#ifdef _DEBUG
	//�f�o�b�N���C���[���I����
	EnableDebugLayer();
#endif // DEBUG

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG

	std::vector<IDXGIAdapter*>adapters;
	IDXGIAdapter* temAdapter = nullptr;
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &temAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {

		adapters.push_back(temAdapter);

	}

	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);
		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			temAdapter = adpt;
			break;
		}

	}

	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) {
			featureLevel = lv;
			break;
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	HRESULT CreateSwapChainForHwnd(
		IUnknown * pDevice,
		HWND hWnd,
		const DXGI_SWAP_CHAIN_DESC1 * pDesc,
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc,
		IDXGIOutput * pRestrictToOutput,
		IDXGISwapChain1 * *ppSwapChain
	);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);

	//SRGB �����_�[�^�[�Q�b�g�r���[�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�K���}�␳�L��
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle
			= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], &rtvDesc, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);

	//���_�f�[�^�\����
	struct Vertex	{
		XMFLOAT3 pos; //xyz���W
		XMFLOAT2 uv;//uv���W
	};


	Vertex vertices[] = {
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}},//����
		{{-0.4f,+0.7f,0.0f},{0.0f,0.0f}},//����
		{{+0.4f,-0.7f,0.0f},{1.0f,1.0f}},//�E��
		{{+0.4f,+0.7f,0.0f},{1.0f,0.0f}},//�E��
	};

	//------------------PGWS(��4��)�`�������W�ۑ�(����)-----------------------//
	//XMFLOAT3 vertices[] = {
	//    {+0.4f,+0.7f,0.0f},//0
	//    {+0.0f,+0.7f,0.0f},//1
	//    {-0.4f,+0.7f,0.0f},//2
	//    {+0.4f,+0.0f,0.0f},//3
	//	  {+0.0f,+0.0f,0.0f},//4
	//	  {-0.4f,+0.0f,0.0f},//5
	//	  {+0.4f,-0.7f,0.0f},//6
	//	  {+0.0f,-0.7f,0.0f},//7
	//	  {-0.4f,-0.7f,0.0f},//8
	//};
	//------------------------------------------------------------//

	/*D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;*/

	ID3D12Resource* vertBuff = nullptr;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourcepDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
	result = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourcepDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	Vertex* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//�o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices);//�S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]);//�P���_������̃o�C�g��

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	unsigned short indices[] = {
		0,1,2,
		2,1,3,
	};

	//-----------------PGWS(��4��)�`�������W�ۑ�(����)----------------------//
	/*unsigned short indices[] = {
		0,3,4,
		4,1,2,
		6,4,7,
		4,5,8
	};*/

	ID3D12Resource* idxBuff = nullptr;
	//�ݒ�́A�o�b�t�@�[�̃T�C�Y�ȊO�A���_�o�b�t�@�[�̐ݒ���g���񂵂Ă悢
	resourcepDesc.Width = sizeof(indices);
	result = _dev->CreateCommittedResource(
	    &heapProp,
	    D3D12_HEAP_FLAG_NONE,
	    &resourcepDesc,
	    D3D12_RESOURCE_STATE_GENERIC_READ,
	    nullptr,
	    IID_PPV_ARGS(&idxBuff));

	//������o�b�t�@�[�ɃC���f�b�N�X�f�[�^�����R�s�[
	unsigned short* mapppedIdx = nullptr;
	idxBuff->Map(0,nullptr,(void**)&mapppedIdx);
	std::copy(std::begin(indices),std::end(indices), mapppedIdx);
	idxBuff->Unmap(0,nullptr);

	//�C���f�b�N�X�o�b�t�@�[�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = sizeof(indices);


	//���_�V�F�[�_�̃R���p�C���R�[�h
	ID3DBlob* errorBlob = nullptr;
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vsBlob, &errorBlob);
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C����������܂���");
		}//�f�o�b�N�E�C���h�E�ւ̃G���[�\��
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);//�s�V�������ȁH
	}

	//�s�N�Z���V�F�[�_�̃R���p�C���R�[�h
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &errorBlob);
	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C����������܂���");
		}//�f�o�b�N�E�C���h�E�ւ̃G���[�\��
		else
		{
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);//�s�V�������ȁH
	}



	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{//uv(�ǉ�)
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,
			0,D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr;

	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	//�f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//�܂��A���`�G�C���A�X�͎g��Ȃ����߁ifalse�j
	gpipeline.RasterizerState.MultisampleEnable = false;

	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����

	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

	gpipeline.NumRenderTargets = 1;//���͈�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//0�`�P�ɐ��K�����ꂽRGBA

	gpipeline.SampleDesc.Count = 1;//�T���v�����O�͂P�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�@�@

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;//�e�N�X�`���͈��
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//��ʂ̓e�N�X�`��
	descTblRange.BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange.OffsetInDescriptorsFromTableStart
		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//�s�N�Z���V�F�[�_�[���猩����
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//�f�B�X�N���v�^�����W�̃A�h���X
	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	//�f�B�X�N���v�^�����W��
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�������̌J��Ԃ�
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�����̌J��Ԃ�
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s���̌J��Ԃ�
	samplerDesc.BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�͍�
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//���`��� LINEAR; 
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
	samplerDesc.MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
	samplerDesc.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_�[���猩����
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//���T���v�����O���Ȃ�

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,//���[�g�V�O�l�`���ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0,//���[�g�V�O�l�`���o�[�W����
		&rootSigBlob,//�V�F�[�_�[��������Ƃ��Ɠ���
		&errorBlob);//�G���[����������

	ID3D12RootSignature* rootsignature = nullptr;
	result = _dev->CreateRootSignature(
		0,//nodemask�B�O�ł悢
		rootSigBlob->GetBufferPointer(),//�V�F�[�_�[�̂Ƃ��Ɠ��l
		rootSigBlob->GetBufferSize(),//�V�F�[�_�[�̂Ƃ��Ɠ��l
		IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release();//�s�v�ɂȂ����̂ŉ��

	gpipeline.pRootSignature = rootsignature;

	ID3D12PipelineState* _pipelinestate = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = window_width;
	viewport.Height = window_height;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;

	D3D12_RECT scissorrect = {};
	scissorrect.top = 0;
	scissorrect.left = 0;
	scissorrect.right = scissorrect.left + window_width;
	scissorrect.bottom = scissorrect.top + window_height;

	//WIC�e�N�X�`��
	TexMetadata metadata = {};
	ScratchImage scratchImag = {};

	result = LoadFromWICFile(
		L"img/textest.png", WIC_FLAGS_NONE,
		&metadata, scratchImag
	);//�wCopyright 2018 Digital-Architex�xflooring_0120.jpg�@textest.png
	auto img = scratchImag.GetImage(0, 0, 0);//���f�[�^���o

	//���ԃo�b�t�@�[�Ƃ��ẴA�b�v���[�h�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapProp = {};

	//�}�b�v�\�ɂ��邽�߁AUPLOAD�ɂ���
	uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	//�A�b�v���[�h�p�Ɏg�p���邱�ƑO��Ȃ̂�UNKNOWN�ł悢�B
	uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	uploadHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�[�̂���()
	uploadHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�[�̂���()

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_UNKNOWN;//�P�Ȃ�f�[�^�̉�Ȃ̂�UNKNOWN
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;//�P�Ȃ�o�b�t�@�[�Ƃ��Ďw��
	resDesc.Width = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)* img->height;//�f�[�^�T�C�Y
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//�A�g�����f�[�^
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//����flag�Ȃ�
	resDesc.SampleDesc.Count = 1;//�ʏ�e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
	resDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

	//���ԃo�b�t�@�[�̍쐬
	ID3D12Resource* uploadbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&uploadHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadbuff)
	);

	//�q�e�N�X�`���̂��߂̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//�q�e�N�X�`���p
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�[�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�[�̂���0

	//���\�[�X�ݒ�(�g���܂킵)
	resDesc.Format = metadata.format;
	resDesc.Width = static_cast<UINT>(metadata.width);//��
	resDesc.Height = static_cast<UINT>(metadata.height);//����
	resDesc.DepthOrArraySize = static_cast<uint16_t>(metadata.arraySize);
	resDesc.MipLevels = static_cast<uint16_t>(metadata.mipLevels);
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�͌��肵�Ȃ�

	ID3D12Resource* texbuff = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,//�R�s�[��
		nullptr,
		IID_PPV_ARGS(&texbuff));

	uint8_t* mapforImg = nullptr;//image->pixels�Ɠ����^�ɂ���
	result = uploadbuff->Map(0, nullptr, (void**)&mapforImg);//�}�b�v

	auto srcAddress = img->pixels;
	auto rowPitch = AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	for (int y = 0; y < img->height; ++y)
	{
		std::copy_n(srcAddress,rowPitch,mapforImg); // �R�s�[

		// 1 �s���Ƃ̂��܂����킹��
		srcAddress += img->rowPitch;
		mapforImg += rowPitch;
	}
	
	//std::copy_n(img->pixels, img->slicePitch, mapforImg);//�R�s�[
	uploadbuff->Unmap(0, nullptr);//�A���}�b�v

	D3D12_TEXTURE_COPY_LOCATION src = {};
	//�R�s�[��(�A�b�v���[�h��)�ݒ�
	src.pResource = uploadbuff;//���ԃo�b�t�@�[
	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	src.PlacedFootprint.Offset = 0;
	src.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadata.width);
	src.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadata.height);
	src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadata.depth);
	src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(img->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	src.PlacedFootprint.Footprint.Format = img->format;

	D3D12_TEXTURE_COPY_LOCATION dst = {};
	//�R�s�[��ݒ�
	dst.pResource = texbuff;
	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dst.SubresourceIndex = 0;

	_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = texbuff;
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	
	_cmdList->ResourceBarrier(1, &BarrierDesc);
	_cmdList->Close();
	
	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	_cmdQueue->Signal(_fence, ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	_cmdAllocator->Release();
	_cmdList->Reset(_cmdAllocator, nullptr);


	result = texbuff->WriteToSubresource(
		0,
		nullptr,//�S�̈�ւ̃R�s�[
		img->pixels,//���̃f�[�^�A�h���X
		static_cast<UINT>(img->rowPitch),//1���C���T�C�Y
		static_cast<UINT>(img->slicePitch)//�S�T�C�Y
	);

	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	//�V�F�[�_�[����͌�����悤��
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//�}�X�N��0
	descHeapDesc.NodeMask = 0;
	//�r���[�͍��̂Ƃ���1����
	descHeapDesc.NumDescriptors = 1;
	//�V�F�[�_�[���\�[�X�r���[�p
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//����
	result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC srcDesc = {};
	srcDesc.Format = metadata.format;
	srcDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
	srcDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srcDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	_dev->CreateShaderResourceView(
		texbuff,//�r���[�Ɗ֘A�t����o�b�t�@�[
		&srcDesc,//��قǐݒ肵���e�N�X�`���ݒ���
		texDescHeap->GetCPUDescriptorHandleForHeapStart()//�q�[�v�̂ǂ������蓖�Ă邩
	);

	//-------------------�ۑ�p ��ڂ̉摜���ڂ��邽�߂̃R�[�h��------------------

	Vertex verticesT[] = {
		{{-0.95f,-0.43f,0.0f},{0.0f,1.0f}},//����
		{{-0.95f,+0.43f,0.0f},{0.0f,0.0f}},//����
		{{-0.45f,-0.43f,0.0f},{1.0f,1.0f}},//�E��
		{{-0.45f,+0.43f,0.0f},{1.0f,0.0f}},//�E��
	};

	ID3D12Resource* vertBuffT = nullptr;

	auto heapPropT = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourcepDescT = CD3DX12_RESOURCE_DESC::Buffer(sizeof(verticesT));
	result = _dev->CreateCommittedResource(
		&heapPropT,
		D3D12_HEAP_FLAG_NONE,
		&resourcepDescT,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuffT));

	Vertex* vertMapT = nullptr;
	result = vertBuffT->Map(0, nullptr, (void**)&vertMapT);
	std::copy(std::begin(verticesT), std::end(verticesT), vertMapT);
	vertBuffT->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbViewT = {};
	vbViewT.BufferLocation = vertBuffT->GetGPUVirtualAddress();//�o�b�t�@�[�̉��z�A�h���X
	vbViewT.SizeInBytes = sizeof(verticesT);//�S�o�C�g��
	vbViewT.StrideInBytes = sizeof(verticesT[0]);//�P���_������̃o�C�g��

	unsigned short indicesT[] = {
		0,1,2,
		2,1,3,
	};

	ID3D12Resource* idxBuffT = nullptr;
	//�ݒ�́A�o�b�t�@�[�̃T�C�Y�ȊO�A���_�o�b�t�@�[�̐ݒ���g���񂵂Ă悢
	resourcepDescT.Width = sizeof(indicesT);
	result = _dev->CreateCommittedResource(
	    &heapPropT,
	    D3D12_HEAP_FLAG_NONE,
	    &resourcepDescT,
	    D3D12_RESOURCE_STATE_GENERIC_READ,
	    nullptr,
	    IID_PPV_ARGS(&idxBuffT));

	//������o�b�t�@�[�ɃC���f�b�N�X�f�[�^�����R�s�[
	unsigned short* mapppedIdxT = nullptr;
	idxBuffT->Map(0,nullptr,(void**)&mapppedIdxT);
	std::copy(std::begin(indicesT),std::end(indicesT), mapppedIdxT);
	idxBuffT->Unmap(0,nullptr);

	//�C���f�b�N�X�o�b�t�@�[�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibViewT = {};
	ibViewT.BufferLocation = idxBuffT->GetGPUVirtualAddress();
	ibViewT.Format = DXGI_FORMAT_R16_UINT;
	ibViewT.SizeInBytes = sizeof(indicesT);


	D3D12_INPUT_ELEMENT_DESC inputLayoutT[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{//uv(�ǉ�)
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,
			0,D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipelineT = {};

	gpipelineT.pRootSignature = nullptr;

	gpipelineT.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipelineT.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipelineT.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipelineT.PS.BytecodeLength = _psBlob->GetBufferSize();

	//�f�t�H���g�̃T���v���}�X�N��\���萔�i0xffffffff�j
	gpipelineT.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	//�܂��A���`�G�C���A�X�͎g��Ȃ����߁ifalse�j
	gpipelineT.RasterizerState.MultisampleEnable = false;

	gpipelineT.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
	gpipelineT.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
	gpipelineT.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����

	gpipelineT.BlendState.AlphaToCoverageEnable = false;
	gpipelineT.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDescT = {};
	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDescT.BlendEnable = false;
	//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
	renderTargetBlendDescT.LogicOpEnable = false;
	renderTargetBlendDescT.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	gpipelineT.BlendState.RenderTarget[0] = renderTargetBlendDescT;

	gpipelineT.InputLayout.pInputElementDescs = inputLayoutT;//���C�A�E�g�擪�A�h���X
	gpipelineT.InputLayout.NumElements = _countof(inputLayoutT);//���C�A�E�g�z��

	gpipelineT.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�

	gpipelineT.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

	gpipelineT.NumRenderTargets = 1;//���͈�̂�
	gpipelineT.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//0�`�P�ɐ��K�����ꂽRGBA

	gpipelineT.SampleDesc.Count = 1;//�T���v�����O�͂P�s�N�Z���ɂ��P
	gpipelineT.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�@�@

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDescT = {};
	rootSignatureDescT.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRangeT = {};
	descTblRangeT.NumDescriptors = 1;//�e�N�X�`���͈��
	descTblRangeT.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//��ʂ̓e�N�X�`��
	descTblRangeT.BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRangeT.OffsetInDescriptorsFromTableStart
		= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparamT = {};
	rootparamT.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//�s�N�Z���V�F�[�_�[���猩����
	rootparamT.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//�f�B�X�N���v�^�����W�̃A�h���X
	rootparamT.DescriptorTable.pDescriptorRanges = &descTblRangeT;
	//�f�B�X�N���v�^�����W��
	rootparamT.DescriptorTable.NumDescriptorRanges = 1;

	rootSignatureDescT.pParameters = &rootparamT;
	rootSignatureDescT.NumParameters = 1;

	D3D12_STATIC_SAMPLER_DESC samplerDescT = {};
	samplerDescT.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�������̌J��Ԃ�
	samplerDescT.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�����̌J��Ԃ�
	samplerDescT.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s���̌J��Ԃ�
	samplerDescT.BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�͍�
	samplerDescT.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//���`��� LINEAR; 
	samplerDescT.MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
	samplerDescT.MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
	samplerDescT.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_�[���猩����
	samplerDescT.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//���T���v�����O���Ȃ�

	rootSignatureDescT.pStaticSamplers = &samplerDescT;
	rootSignatureDescT.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlobT = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDescT,//���[�g�V�O�l�`���ݒ�
		D3D_ROOT_SIGNATURE_VERSION_1_0,//���[�g�V�O�l�`���o�[�W����
		&rootSigBlobT,//�V�F�[�_�[��������Ƃ��Ɠ���
		&errorBlob);//�G���[����������

	ID3D12RootSignature* rootsignatureT = nullptr;
	result = _dev->CreateRootSignature(
		0,//nodemask�B�O�ł悢
		rootSigBlobT->GetBufferPointer(),//�V�F�[�_�[�̂Ƃ��Ɠ��l
		rootSigBlobT->GetBufferSize(),//�V�F�[�_�[�̂Ƃ��Ɠ��l
		IID_PPV_ARGS(&rootsignatureT));
	rootSigBlobT->Release();//�s�v�ɂȂ����̂ŉ��

	gpipelineT.pRootSignature = rootsignatureT;

	ID3D12PipelineState* _pipelinestateT = nullptr;
	result = _dev->CreateGraphicsPipelineState(&gpipelineT, IID_PPV_ARGS(&_pipelinestateT));

	D3D12_VIEWPORT viewportT = {};
	viewportT.Width = window_width;
	viewportT.Height = window_height;
	viewportT.TopLeftX = 0;
	viewportT.TopLeftY = 0;
	viewportT.MaxDepth = 1.0f;
	viewportT.MinDepth = 0.0f;

	D3D12_RECT scissorrectT = {};
	scissorrectT.top = 0;
	scissorrectT.left = 0;
	scissorrectT.right = scissorrectT.left + window_width;
	scissorrectT.bottom = scissorrectT.top + window_height;

	//WIC�e�N�X�`��
	TexMetadata metadataT = {};
	ScratchImage scratchImagT = {};

	result = LoadFromWICFile(
		L"img/flooring_0120.jpg", WIC_FLAGS_NONE,
		&metadataT, scratchImagT
	);//flooring_0120.jpg�wCopyright 2018 Digital-Architex�x
	auto imgT = scratchImagT.GetImage(0, 0, 0);//���f�[�^���o

	//���ԃo�b�t�@�[�Ƃ��ẴA�b�v���[�h�q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES uploadHeapPropT = {};

	//�}�b�v�\�ɂ��邽�߁AUPLOAD�ɂ���
	uploadHeapPropT.Type = D3D12_HEAP_TYPE_UPLOAD;

	//�A�b�v���[�h�p�Ɏg�p���邱�ƑO��Ȃ̂�UNKNOWN�ł悢�B
	uploadHeapPropT.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	uploadHeapPropT.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	uploadHeapPropT.CreationNodeMask = 0;//�P��A�_�v�^�[�̂���()
	uploadHeapPropT.VisibleNodeMask = 0;//�P��A�_�v�^�[�̂���()

	D3D12_RESOURCE_DESC resDescT = {};
	resDescT.Format = DXGI_FORMAT_UNKNOWN;//�P�Ȃ�f�[�^�̉�Ȃ̂�UNKNOWN
	resDescT.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;//�P�Ȃ�o�b�t�@�[�Ƃ��Ďw��
	resDescT.Width = AlignmentedSize(imgT->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)* imgT->height;//�f�[�^�T�C�Y
	resDescT.Height = 1;
	resDescT.DepthOrArraySize = 1;
	resDescT.MipLevels = 1;
	resDescT.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//�A�g�����f�[�^
	resDescT.Flags = D3D12_RESOURCE_FLAG_NONE;//����flag�Ȃ�
	resDescT.SampleDesc.Count = 1;//�ʏ�e�N�X�`���Ȃ̂ŃA���`�G�C���A�V���O���Ȃ�
	resDescT.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

	//���ԃo�b�t�@�[�̍쐬
	ID3D12Resource* uploadbuffT = nullptr;
	result = _dev->CreateCommittedResource(
		&uploadHeapPropT,
		D3D12_HEAP_FLAG_NONE,
		&resDescT,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadbuffT)
	);

	//�q�e�N�X�`���̂��߂̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapPropT = {};
	texHeapPropT.Type = D3D12_HEAP_TYPE_DEFAULT;//�q�e�N�X�`���p
	texHeapPropT.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	texHeapPropT.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	texHeapPropT.CreationNodeMask = 0;//�P��A�_�v�^�[�̂���0
	texHeapPropT.VisibleNodeMask = 0;//�P��A�_�v�^�[�̂���0

	//���\�[�X�ݒ�(�g���܂킵)
	resDescT.Format = metadataT.format;
	resDescT.Width = static_cast<UINT>(metadataT.width);//��
	resDescT.Height = static_cast<UINT>(metadataT.height);//����
	resDescT.DepthOrArraySize = static_cast<uint16_t>(metadataT.arraySize);
	resDescT.MipLevels = static_cast<uint16_t>(metadataT.mipLevels);
	resDescT.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadataT.dimension);
	resDescT.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�͌��肵�Ȃ�

	ID3D12Resource* texbuffT = nullptr;
	result = _dev->CreateCommittedResource(
		&texHeapPropT,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDescT,
		D3D12_RESOURCE_STATE_COPY_DEST,//�R�s�[��
		nullptr,
		IID_PPV_ARGS(&texbuffT));

	uint8_t* mapforImgT = nullptr;//image->pixels�Ɠ����^�ɂ���
	result = uploadbuffT->Map(0, nullptr, (void**)&mapforImgT);//�}�b�v

	auto srcAddressT = imgT->pixels;
	auto rowPitchT = AlignmentedSize(imgT->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	for (int y = 0; y < imgT->height; ++y)
	{
		std::copy_n(srcAddressT,rowPitchT,mapforImgT); // �R�s�[

		// 1 �s���Ƃ̂��܂����킹��
		srcAddressT += imgT->rowPitch;
		mapforImgT += rowPitchT;
	}
	
	uploadbuffT->Unmap(0, nullptr);//�A���}�b�v

	D3D12_TEXTURE_COPY_LOCATION srcT = {};
	//�R�s�[��(�A�b�v���[�h��)�ݒ�
	srcT.pResource = uploadbuffT;//���ԃo�b�t�@�[
	srcT.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	srcT.PlacedFootprint.Offset = 0;
	srcT.PlacedFootprint.Footprint.Width = static_cast<UINT>(metadataT.width);
	srcT.PlacedFootprint.Footprint.Height = static_cast<UINT>(metadataT.height);
	srcT.PlacedFootprint.Footprint.Depth = static_cast<UINT>(metadataT.depth);
	srcT.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(AlignmentedSize(imgT->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT));
	srcT.PlacedFootprint.Footprint.Format = imgT->format;

	D3D12_TEXTURE_COPY_LOCATION dstT = {};
	//�R�s�[��ݒ�
	dstT.pResource = texbuffT;
	dstT.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	dstT.SubresourceIndex = 0;

	_cmdList->CopyTextureRegion(&dstT, 0, 0, 0, &srcT, nullptr);

	D3D12_RESOURCE_BARRIER BarrierDescT = {};
	BarrierDescT.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDescT.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDescT.Transition.pResource = texbuffT;
	BarrierDescT.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDescT.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	BarrierDescT.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	
	_cmdList->ResourceBarrier(1, &BarrierDescT);
	_cmdList->Close();
	
	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlistsT[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlistsT);

	_cmdQueue->Signal(_fence, ++_fenceVal);
	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator, nullptr);


	result = texbuffT->WriteToSubresource(
		0,
		nullptr,//�S�̈�ւ̃R�s�[
		imgT->pixels,//���̃f�[�^�A�h���X
		static_cast<UINT>(imgT->rowPitch),//1���C���T�C�Y
		static_cast<UINT>(imgT->slicePitch)//�S�T�C�Y
	);

	ID3D12DescriptorHeap* texDescHeapT = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDescT = {};
	//�V�F�[�_�[����͌�����悤��
	descHeapDescT.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//�}�X�N��0
	descHeapDescT.NodeMask = 0;
	//�r���[�͍��̂Ƃ���1����
	descHeapDescT.NumDescriptors = 1;
	//�V�F�[�_�[���\�[�X�r���[�p
	descHeapDescT.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//����
	result = _dev->CreateDescriptorHeap(&descHeapDescT, IID_PPV_ARGS(&texDescHeapT));

	D3D12_SHADER_RESOURCE_VIEW_DESC srcDescT = {};
	srcDescT.Format = metadataT.format;
	srcDescT.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
	srcDescT.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srcDescT.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	_dev->CreateShaderResourceView(
		texbuffT,//�r���[�Ɗ֘A�t����o�b�t�@�[
		&srcDescT,//��قǐݒ肵���e�N�X�`���ݒ���
		texDescHeapT->GetCPUDescriptorHandleForHeapStart()//�q�[�v�̂ǂ������蓖�Ă邩
	);
	//---------------------------------------------------------------------------------------

	//int color_number = 0;

	/*MSG msg = {};*/
	while (true)
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		/*TranslateMessage(&msg);
		DispatchMessage(&msg);*/

		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

		_cmdList->SetPipelineState(_pipelinestateT);

		/*D3D12_RESOURCE_BARRIER BarrierDesc = {};
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = 0;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;*/

		auto BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx], D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		_cmdList->ResourceBarrier(1, &BarrierDesc);

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		float clearColor[] = { 1.0f,0.5f, 0.0f, 1.0f };//�D�F

		////�`�J�`�J
		//if (color_number >= 0&& color_number < 10 ) {
		   // //clearColor[] = { 1.0f,0.0f, 0.0f, 1.0f};//��
		   // clearColor[0] = { 1.0f};
		   // clearColor[1] = { 0.0f};
		   // clearColor[2] = { 0.0f};
		   // clearColor[3] = { 1.0f};
		   // color_number++;
		//}
		//else if (color_number >= 10 && color_number < 20) {
		   // //float clearColor[] = { 1.0f,1.0f, 0.0f, 1.0f };//���F
		   // clearColor[0] = { 1.0f };
		   // clearColor[1] = { 1.0f };
		   // clearColor[2] = { 0.0f };
		   // clearColor[3] = { 1.0f };
		   // color_number++;
		//}
		//else if (color_number >= 20 && color_number < 30) {
		   // //float clearColor[] = { 0.0f,0.0f, 1.0f, 1.0f };//��
		   // clearColor[0] = { 0.0f };
		   // clearColor[1] = { 0.0f };
		   // clearColor[2] = { 1.0f };
		   // clearColor[3] = { 1.0f };
		   // color_number++;
		//}
		//else if (color_number >= 30 && color_number < 40) {
		   // //float clearColor[] = { 0.0f,1.0f, 1.0f, 1.0f };//��
		   // clearColor[0] = { 0.0f };
		   // clearColor[1] = { 1.0f };
		   // clearColor[2] = { 1.0f };
		   // clearColor[3] = { 1.0f };
		   // color_number++;
		//}
		//else if (color_number >= 40) {
		   // color_number = 0;
		//}

		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		/*BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;*/
		BarrierDesc = CD3DX12_RESOURCE_BARRIER::Transition(
			_backBuffers[bbIdx], D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		_cmdList->SetGraphicsRootSignature(rootsignature);
		_cmdList->SetDescriptorHeaps(1, &texDescHeap);
		_cmdList->SetGraphicsRootDescriptorTable(
			0,texDescHeap->GetGPUDescriptorHandleForHeapStart());
		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
		_cmdList->IASetIndexBuffer(&ibView);
		_cmdList->DrawIndexedInstanced(6/*12*/, 1, 0, 0, 0);

		//-------------------�ۑ�p ��ڂ̉摜���ڂ��邽�߂̃R�[�h��------------------

		_cmdList->SetGraphicsRootSignature(rootsignatureT);
		_cmdList->SetDescriptorHeaps(1, &texDescHeapT);
		_cmdList->SetGraphicsRootDescriptorTable(
			0, texDescHeapT->GetGPUDescriptorHandleForHeapStart());
		_cmdList->RSSetViewports(1, &viewportT);
		_cmdList->RSSetScissorRects(1, &scissorrectT);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbViewT);
		_cmdList->IASetIndexBuffer(&ibViewT);
		_cmdList->DrawIndexedInstanced(6/*12*/, 1, 0, 0, 0);

		//------------------------------------------------------------------------------

		_cmdList->Close();

		ID3D12CommandList * cmdlists[] = { _cmdList };
		_cmdQueue->ExecuteCommandLists(1, cmdlists);

		_cmdQueue->Signal(_fence, ++_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}


		_cmdAllocator->Reset();
		_cmdList->Reset(_cmdAllocator, nullptr);

		_swapchain->Present(1, 0);
	}

	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}
