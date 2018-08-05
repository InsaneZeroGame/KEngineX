#pragma once

namespace KFramework
{
    class IModule
    {
    public:
        IModule()
        {
        }
        virtual ~IModule()
        {
        }
        virtual void Init() = 0;
        virtual void Update() = 0;
        virtual void Destory() = 0;
    };//IModule
}//KFramework
