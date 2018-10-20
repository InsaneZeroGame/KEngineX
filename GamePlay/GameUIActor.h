#pragma once
#include "GameActor.h"


namespace gameplay
{
    class GameUIActor : public GameRenderActor
    {
    public:
        GameUIActor(const std::string& p_name);

        ~GameUIActor();

        void SetShow(bool p_show);

        bool IsShow();

    private:
        bool m_show = false;
    };
}
