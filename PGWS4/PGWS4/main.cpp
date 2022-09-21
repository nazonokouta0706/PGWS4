#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<vector>
#ifdef _DEBUG
#include<iostream>
#endif


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace std;

void DebugOutFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist,format);
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

int main()
{
#else
 int WINAPI WinMain(HINSTANCE<HINSTANCE,LPSTR,int)
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

	 D3D_FEATURE_LEVEL levels[] = {
		 D3D_FEATURE_LEVEL_12_1,
		 D3D_FEATURE_LEVEL_12_0,
		 D3D_FEATURE_LEVEL_11_1,
		 D3D_FEATURE_LEVEL_11_0,
	 };

	 auto result = CreateDXGIFactory(IID_PPV_ARGS(&_dxgiFactory));

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
	 result = _dev->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,
		 _cmdAllocator,nullptr,IID_PPV_ARGS(&_cmdList));

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

	 std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	 for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	 {
		 result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		 D3D12_CPU_DESCRIPTOR_HANDLE handle
			 = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		 handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			 D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		 _dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	 }

	 ShowWindow(hwnd, SW_SHOW);

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

		 auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		 rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
			 D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		 );
		 _cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		 float clearColor[] = { 1.0f,1.0f, 0.0f, 1.0f };//���F
		 _cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		 _cmdList->Close();

		 ID3D12CommandList* cmdlists[] = { _cmdList };
		 _cmdQueue->ExecuteCommandLists(1, cmdlists);

		 _cmdAllocator->Reset();
		 _cmdList->Reset(_cmdAllocator, nullptr);

		 _swapchain->Present(1, 0);
	 }

	 UnregisterClass(w.lpszClassName, w.hInstance);

	 return 0;
}
