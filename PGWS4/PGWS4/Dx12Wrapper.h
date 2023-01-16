#pragma once
#include <Windows.h>
#include <vector>
#include <map>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <DirectXTex.h>
#include <wrl.h>

class Dx12Wrapper {

private:
	Microsoft::WRL::ComPtr<ID3D12Device> _dev = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory6>_dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapchain = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence> _fence = nullptr;
	UINT64 _fenceVal = 0;

	unsigned int _screen_size[2];
	Microsoft::WRL::ComPtr<ID3D12Resource> _depthBuffer = nullptr;
	std::vector<ID3D12Resource*> _backBuffers ;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap = nullptr;

	CD3DX12_VIEWPORT _viewport;
	CD3DX12_RECT _scissorrect;

	std::map<std::string, Microsoft::WRL::ComPtr<ID3D12Resource>> _textureTable;
	using LoadLambda_t = std::function<HRESULT(const std::wstring& path, DirectX::TexMetadata*, DirectX::ScratchImage&)>;
	std::map<std::string, LoadLambda_t> _loadLambdaTable;

	DirectX::XMMATRIX _viewMat;
	DirectX::XMMATRIX _projMat;

	//�V�F�[�_�[���ɓn�����߂̊�{�I�ȍs��f�[�^
	struct SceneData
	{
		DirectX::XMMATRIX view;//�r���[�s��
		DirectX::XMMATRIX proj;//�v���W�F�N�V�����s��
		DirectX::XMFLOAT3 eye;//���_���W
	};
	SceneData* _mapScene; //�}�b�v��������|�C���^�[
	Microsoft::WRL::ComPtr<ID3D12Resource> _sceneConstBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _sceneDescHeap = nullptr;

private:

	//�������֌W�̊֐���
	static Microsoft::WRL::ComPtr<IDXGIFactory6> InitializeGraphicsInterface();
	static Microsoft::WRL::ComPtr<ID3D12Device> InitializeDevice(IDXGIFactory6* dxgiFactory);

	static void InitializeCommand(Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& cmdAllocator,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& cmdList,
		Microsoft::WRL::ComPtr<ID3D12CommandQueue>& cmdQueue, ID3D12Device* dev);

	static Microsoft::WRL::ComPtr<IDXGISwapChain4> CreateSwapChain(unsigned int width, unsigned int height,
		IDXGIFactory6* dxgiFactory, ID3D12CommandQueue* cmdQueue, HWND hwnd);

	static void InitializeBackBuffers(std::vector<ID3D12Resource*>& backBuffers, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& rtvHeaps,
		ID3D12Device* dev, IDXGISwapChain4* swapchain);

	void InitializeDepthBuffer(Microsoft::WRL::ComPtr<ID3D12Resource>& depthBuffer, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& dsvHeap,
		unsigned int width, unsigned int height, ID3D12Device* dev);

	UINT64 InitializeFence(Microsoft::WRL::ComPtr<ID3D12Fence>& fence, ID3D12Device* dev);

	void InitializeMatrixes();
	void CreateSceneTransformView();

	Microsoft::WRL::ComPtr<ID3D12Resource> LoadTextureFromFile(const char* texPath);
	void InitializeTextureLoaderTable(std::map<std::string, LoadLambda_t>& loadLambdaTable);

public:
	Dx12Wrapper(HWND hwnd, unsigned int width, unsigned int height);
	~Dx12Wrapper();

	void BeginDraw();
	void EndDraw();	

	void ApplySceneDescHeap();

	Microsoft::WRL::ComPtr<ID3D12Device> Device() { return _dev; }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList() { return _cmdList; }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTextureByPath(const char* texpath);

};
