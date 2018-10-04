#pragma once
#include "Line.h"

namespace Math
{

    class Grid : public Primitive
    {
    public:
        Grid(float size, uint32_t count, const Color& p_color = {0.8f,0.8f,0.8f}) :
            m_size(size),
            m_count(count)
        {
            m_color = p_color;

            std::vector<Line> l_lines;

            float line_step = 2 * m_size / float(m_count);

            for (float y = -m_size; y <= m_size; y += line_step)
            {
                l_lines.push_back(Line(Vector3(m_size,  0.0f,y ), Vector3(-m_size , 0.0f, y), m_color));
            }

            for (float x = -m_size; x <= m_size; x += line_step)
            {
                l_lines.push_back(Line(Vector3(x, 0.0f, m_size), Vector3(x, 0.0f, -m_size), m_color));

            }
            int line_index = 0;
            for (auto& l_line : l_lines)
            {
                auto l_line_vertices = l_line.GetVertices();
                m_vertices.push_back(l_line_vertices[0]);
                m_vertices.push_back(l_line_vertices[1]);
                m_indices.push_back(line_index++);
                m_indices.push_back(line_index++);
            }

            
        };
        ~Grid()
        {
            
        };
        
    private:
        float m_size;
        uint32_t m_count;

    };

    
}