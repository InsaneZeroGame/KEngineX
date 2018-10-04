#pragma once
#include "Vector.h"
#include <vector>
#include <EngineConfig.h>

namespace Math
{

    using Point = Vector3;
    using Color = Vector3;

    class Primitive
    {
    public:
        Primitive() :m_vertices({}), m_indices({})
        {
            
        };
        virtual ~Primitive()
        {
        
        };

        __forceinline const std::vector<Renderer::Vertex>& GetVertices() const
        {
            return m_vertices;
        };

        __forceinline const std::vector<uint32_t>& GetIndices() const
        {
            return m_indices;
        };

        __forceinline const Color& GetColor() const
        {
            return m_color;
        };

    protected:
        Color m_color;

        std::vector<Renderer::Vertex> m_vertices;
        
        std::vector<uint32_t> m_indices;
    };
    
}