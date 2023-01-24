#include "Application.h"
#include<tchar.h>
//#include<assert.h>
//#include<vector>
//#include<exception>
//#include<d3dcompiler.h>
#include "Dx12Wrapper.h"
#include "PMDRenderer.h"
#include "PMDActor.h"

//#ifdef _DEBUG
//#include<iostream>
//#endif

using namespace DirectX;
using namespace Microsoft::WRL;

static void DebugOutFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}

static LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); // OS �ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // ����̏������s��
}

static HWND CreateGameWindow(WNDCLASSEX& _wc, UINT width, UINT height)
{
	_wc.cbSize = sizeof(WNDCLASSEX);
	_wc.lpfnWndProc = (WNDPROC)WindowProcedure;
	_wc.lpszClassName = _T("DX12Sample");
	_wc.hInstance = GetModuleHandle(nullptr);

	RegisterClassEx(&_wc);// �A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w��� OS �ɓ`����j

	RECT wrc = { 0,0,(LONG)width,(LONG)height };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	return CreateWindow(_wc.lpszClassName,
		_T("DX12Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		_wc.hInstance,
		nullptr);
}

bool Application::Init()
{
	ThrowIfFailed(CoInitializeEx(0, COINIT_MULTITHREADED));

	_hwnd = CreateGameWindow(_wc, _window_width, _window_height);// �E�B���h�E����

	_dx12.reset(new Dx12Wrapper(_hwnd, _window_width, _window_height));
	_pmdRenderer.reset(new PMDRenderer(*_dx12));
	//  Model/�������J.pmd �����~�Nmetal.pmd �����~�NVer2.pmd �����~�N.pmd
	_pmdActor.reset(new PMDActor("Model/�����~�Nmetal.pmd", *_pmdRenderer));
	//motion/pose.vmd  motion/swing.vmd  motion/motion.vmd motion/avs.vmd  
	_pmdActor->LoadVMDFile("motion/good3.vmd", "pose"); //���̃A�j���[�V����(�O�b�W���u�|�[�Y)
	_pmdActor->PlayAnimaton();

	return true;
}

void Application::Run() 
{
	ShowWindow(_hwnd, SW_SHOW);

	while (true)
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}

		_pmdActor->Update();
		_dx12->BeginDraw();

		ComPtr<ID3D12GraphicsCommandList> _cmdList = _dx12->CommandList();
		_cmdList->SetPipelineState(_pmdRenderer->GetPipelineState());
		_cmdList->SetGraphicsRootSignature(_pmdRenderer->GetRootSignature());
		_dx12->ApplySceneDescHeap();
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_pmdActor->Draw();
		_dx12->EndDraw();
	}
}

void Application::Terminate()
{
	UnregisterClass(_wc.lpszClassName, _wc.hInstance);
}

Application::Application()
{
}

Application::~Application()
{
}

Application& Application::Instance()
{
	static Application instance;
	return instance;
}