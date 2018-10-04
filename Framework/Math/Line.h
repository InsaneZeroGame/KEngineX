#pragma once
#include "Primitive.h"

namespace Math
{
   

    class Line : public Primitive
    {

    public:

       Line(const Point& p_a, const Point& p_b, const Color& p_color = {0.8f,0.8f,0.8f}) 
           :a(p_a), 
           b(p_b)
       {
           m_color = p_color;

           Renderer::Vertex v_a = {};
           v_a.m_ui_color_vertex.position = { a.GetX(),a.GetY(),a.GetZ() };
           v_a.m_ui_color_vertex.color = { m_color.GetX(),m_color.GetY(),m_color.GetZ() };
           v_a.m_ui_color_vertex.padding = {};

           Renderer::Vertex v_b = {};
           v_b.m_ui_color_vertex.position = { b.GetX(),b.GetY(),b.GetZ(), };
           v_b.m_ui_color_vertex.color = { m_color.GetX(),m_color.GetY(),m_color.GetZ() };
           v_b.m_ui_color_vertex.padding = {};

           m_vertices = { v_a ,v_b};

           m_indices = {0,1};

       }

    protected:

        Point a;
        Point b;

      
    };

}