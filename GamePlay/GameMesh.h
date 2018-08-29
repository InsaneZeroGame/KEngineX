#pragma once
#include <vector>


namespace gameplay
{
    struct GameMesh
    {
    public:
        GameMesh();

        GameMesh(std::vector<float> p_vertices,std::vector<uint32_t> p_indices);

        ~GameMesh();

        struct MeshDescriptor
        {
            //To Support All graphics.
            uint64_t BufferLocation;//D3D12 Buffer View
            uint32_t SizeInBytes;   //D3D12 Buffer View
            uint32_t StrideInBytes; //D3D12 Buffer View

        } m_vertex_buffer_desc;


        MeshDescriptor m_index_buffer_desc;

        std::vector<uint32_t> m_indices;

        uint32_t m_index_count = 0;

        std::vector<float> m_vertices;
        
    };//class GameMesh



    struct GameMeterial
    {
        //Material has multiple meshes shares the same material(diffuse,specular...)
    public:
        GameMeterial();

        ~GameMeterial();

        std::vector<GameMesh> m_meshes;
    };//Class GameMaterial 


    
}//namespace gameplay