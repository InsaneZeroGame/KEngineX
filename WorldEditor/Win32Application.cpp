//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "Win32Application.h"
#include "ObjLoaderHelper.h"
#include "GameInput.h"
#include <GameDirector.h>
#include <fbxsdk.h>


HWND Win32Application::m_hwnd = nullptr;
Renderer::IRenderer* Win32Application::m_dx12_renderer = nullptr;




static std::shared_ptr<gameplay::GamesScene> PrepareADummyScene() 
{
    assert(0);
    //std::vector<float> triangleVertices =
    //{
    //   0.0f, 0.25f, 0.0f    , 1.0f, 0.0f, 0.0f, 1.0f ,
    //   0.25f, -0.25f, 0.0f  , 0.0f, 1.0f, 0.0f, 1.0f ,
    //   -0.25f, -0.25f, 0.0f , 0.0f, 0.0f, 1.0f, 1.0f ,
    //};
    //
    //using namespace gameplay;
    //auto dummy_scene = std::make_shared<gameplay::GamesScene>();
    //auto dummy_material = std::make_shared<GameMeterial>();
    //GameMesh dummy_mesh1({
    //    0.0f, 1.0f, 0.0f    , 1.0f, 0.0f, 0.0f, 1.0f ,
    //    1.0f, -1.0f, 0.0f  , 0.0f, 1.0f, 0.0f, 1.0f ,
    //    -1.0f, -1.0f, 0.0f , 0.0f, 0.0f, 1.0f, 1.0f ,
    //    }, { 0,1,2 });
    //GameMesh dummy_mesh2({
    //    0.25f + 0.0f, 0.25f, 0.0f    , 1.0f, 0.0f, 0.0f, 1.0f ,
    //    0.25f + 0.25f, -0.25f, 0.0f  , 0.0f, 1.0f, 0.0f, 1.0f ,
    //    0.25f + -0.25f, -0.25f, 0.0f , 0.0f, 0.0f, 1.0f, 1.0f ,
    //    }, { 0,1,2 });
    //
    //
    //dummy_material->m_meshes.push_back(dummy_mesh1);
    //dummy_material->m_meshes.push_back(dummy_mesh2);
    //
    //dummy_scene->dummy_actor->AddMaterial(dummy_material);
    //dummy_material.reset();
    //return dummy_scene;
}




int Win32Application::Run(HINSTANCE hInstance, int nCmdShow)
{

	// Initialize the window class.
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "KEngine";
	RegisterClassEx(&windowClass);

	//AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_hwnd = CreateWindow(
		windowClass.lpszClassName,
		"KEngine",
		WS_OVERLAPPEDWINDOW,
        KEngineConstants::WINDOW_X,
        KEngineConstants::WINDOW_Y,
        KEngineConstants::WINDOW_WIDTH,
        KEngineConstants::WINDOW_HEIGHT,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		nullptr);

	// Initialize the sample. OnInit is defined in each child-implementation of DXSample.

	ShowWindow(m_hwnd, nCmdShow);
    GameInput::Initialize(m_hwnd);

    //Init Renderer
    m_dx12_renderer = new Renderer::DX12Renderer();
    m_dx12_renderer->SetWindow(m_hwnd, KEngineConstants::WINDOW_X,KEngineConstants::WINDOW_Y, KEngineConstants::WINDOW_WIDTH, KEngineConstants::WINDOW_HEIGHT);
    auto& game_director = gameplay::GameDirector::GetGameDirector();
    m_dx12_renderer->Init();

    game_director.SetRenderer(m_dx12_renderer);
    game_director.Update();

    //Init GameInput
    static int i = 0;

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
        m_dx12_renderer->Update();
        //GameInput::Update(i++);
        //GameInput::GetDurationPressed(GameInput::DigitalInput::kKey_e);
		// Process any messages in the queue.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    m_dx12_renderer->Destory();
	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK Win32Application::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_CREATE:break;
    case WM_KEYDOWN:break;
	case WM_KEYUP:break;
    case WM_PAINT: break;
	case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
	}

	// Handle any messages the switch statement didn't.
	return DefWindowProc(hWnd, message, wParam, lParam);
}
