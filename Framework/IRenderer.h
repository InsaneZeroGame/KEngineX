#pragma once
#include "INoCopy.h"
#include "IModule.h"
#include <GameScene.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <stdint.h>
#include <windows.h>

namespace Renderer
{
    class IRenderer : public KFramework::IModule,public KFramework::INoCopy
    {
    public:
        IRenderer() :
            m_hwnd(nullptr),
            m_window_height(0),
            m_window_width(0)
        {
        
        }
        virtual ~IRenderer()
        {
        }

        // Inherited via IModule
        virtual void Init() override
        {
        }

        virtual void Update() override
        {
        }

        virtual void Destory() override
        {
        }

        virtual void LoadScene(std::shared_ptr<gameplay::GamesScene>) = 0;
        virtual void SetWindow(HWND hWnd, uint32_t height, uint32_t width) = 0;
    protected:
        HWND m_hwnd;

        uint32_t m_window_height;

        uint32_t m_window_width;
    };
    
}