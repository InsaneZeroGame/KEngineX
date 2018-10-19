#pragma once
#include "Primitive.h"
#include "Line.h"


namespace Math
{
    class BoundingBox final: public Primitive
    {
    public:
        BoundingBox(const Point& max,const Point& min,
            const Color& p_color = { 0.0f,1.0f,0.0f }) :
            Primitive(p_color)
        {
            m_vertices.resize(BoundingBoxVerticesCount);
            m_vertices[0].m_ui_color_vertex.position = { max.GetX(),max.GetY(), max.GetZ(),};
            m_vertices[1].m_ui_color_vertex.position = { max.GetX(),min.GetY(), max.GetZ(), };
            m_vertices[2].m_ui_color_vertex.position = { max.GetX(),min.GetY(), min.GetZ(), };
            m_vertices[3].m_ui_color_vertex.position = { max.GetX(),max.GetY(), min.GetZ(), };
            m_vertices[4].m_ui_color_vertex.position = { min.GetX(),min.GetY(), min.GetZ(), };
            m_vertices[5].m_ui_color_vertex.position = { min.GetX(),min.GetY(), max.GetZ(), };
            m_vertices[6].m_ui_color_vertex.position = { min.GetX(),max.GetY(), max.GetZ(), };
            m_vertices[7].m_ui_color_vertex.position = { min.GetX(),max.GetY(), min.GetZ(), };

            
            for (auto& vertex: m_vertices)
            {
                vertex.m_ui_color_vertex.color = { p_color.GetX(),p_color.GetY(), p_color.GetZ(),1.0f };
                vertex.m_ui_color_vertex.padding = {};
            }

            m_indices = {
                         0,1,1,2,2,3,3,0,
                         4,5,5,6,6,7,7,4,
                         0,6,1,5,2,4,3,7
            };
        };
        ~BoundingBox()
        {
            
        };
    private:
        enum 
        {
            BoundingBoxVerticesCount = 8,
        };
    };
}