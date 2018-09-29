#pragma once
#include <EngineConfig.h>
#include <array>
#include "Vector.h"

namespace Math
{

    //
    //     top-left(0)---------------top-right(3)
    //        |                             |
    //        |                             |
    //        |                             |
    //        |                             |
    //        |                             |
    //        |                             |
    //        |                             |
    //     bottom-left(1)----------bottom-right(2)
    struct Rect
    {
    public:
        Rect() :m_pos({})
        {
        
        }

        Rect(Vector3 p_top_left,float width,float height)
        {
            p_top_left.SetZ(0.0f);
            m_pos[0] = p_top_left;
            m_pos[1] = Vector3(p_top_left.GetX(), p_top_left.GetY() - height,0.0f);
            m_pos[2] = Vector3(p_top_left.GetX() + width, p_top_left.GetY() - height, 0.0f);
            m_pos[3] = Vector3(p_top_left.GetX() + width, p_top_left.GetY(), 0.0f);

        }

        Rect(Vector3 p_top_left, float width):Rect(p_top_left, width, width)
        {
            
        }

        ~Rect()
        {
            
        }

        std::array<Vector3, 4> m_pos;
    };

    
}