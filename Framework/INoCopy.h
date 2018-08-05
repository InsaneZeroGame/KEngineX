#pragma once

namespace KFramework {
    class INoCopy
    {
    public:
        INoCopy()
        {
        }
        virtual ~INoCopy()
        {
        }
        INoCopy(const INoCopy&) = delete;
        INoCopy& operator=(const INoCopy&) = delete;
    };//INoCopy
}//KFramework