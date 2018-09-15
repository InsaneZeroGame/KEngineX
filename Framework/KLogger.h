#pragma once
#include "INoCopy.h"
#include "IModule.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>


#define KLOG_INFO KFramework::KLogger::GetLogger()->info
#define KLOG_WARN KFramework::KLogger::GetLogger()->warn
#define KLOG_CRITICAL KFramework::KLogger::GetLogger()->critical

namespace KFramework
{

    class KLogger : public INoCopy
    {
    public:
        static std::shared_ptr<spdlog::logger> GetLogger()
        {
            static  KLogger l_logger;
            return l_logger.m_logger;
        };

        ~KLogger();

    private:
        KLogger();
        std::shared_ptr<spdlog::logger> m_logger;
    };

}