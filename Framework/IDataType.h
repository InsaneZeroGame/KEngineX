#pragma once
#include <vector>

namespace KFramework
{
    class IGPUStatic
    {
    public:


        IGPUStatic(const std::string& p_name):m_name(p_name)
        {
            m_data.push_back(this);
        };
        virtual ~IGPUStatic()
        {

        };

        static void ReleaseAllData()
        {
            for (auto data : m_data)
            {
                if (!data->IsReleased())
                {
                    data->ReleaseData();
                }
            }
        };

        virtual void ReleaseData() = 0;

        __forceinline void SetReleased()
        {
            m_released = true;
        }

        __forceinline bool IsReleased() const
        {
            return m_released;
        }
    protected:
        bool m_released = false;
    private:
        static std::vector<IGPUStatic*> m_data;

        std::string m_name;

    };
}
