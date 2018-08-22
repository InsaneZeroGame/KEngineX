#pragma once
#include <vector>


namespace gameplay
{
  
    struct GameSubMesh
    {
    public:
        GameSubMesh();

        ~GameSubMesh();

        std::vector<uint64_t> m_indices;
    };

    struct GameMesh
    {
    public:
        GameMesh();

        ~GameMesh();

        void AddSubMesh(GameSubMesh&& p_mesh)
        {
            m_sub_meshes.push_back(p_mesh);
        }

        struct MeshDescriptor
        {
            //To Support All graphics.
            uint64_t BufferLocation;//D3D12 Buffer View
            uint32_t SizeInBytes;   //D3D12 Buffer View
            uint32_t StrideInBytes; //D3D12 Buffer View

        } m_mesh_desc;

        std::vector<float> m_vertices;
        std::vector<GameSubMesh> m_sub_meshes;
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