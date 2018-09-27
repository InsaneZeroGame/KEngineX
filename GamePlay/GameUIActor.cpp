#include "GameUIActor.h"

gameplay::GameUIActor::GameUIActor(const std::string& p_name):
    GameActor(p_name)
{

}

void gameplay::GameUIActor::SetShow(bool p_show)
{
    m_show = p_show;
}

bool gameplay::GameUIActor::IsShow()
{
    return m_show;
}

gameplay::GameUIActor::~GameUIActor()
{
}
