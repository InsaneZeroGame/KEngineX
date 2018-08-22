#include "GameMesh.h"


namespace gameplay
{

    //Game Mesh contains Vertices.
    GameMesh::GameMesh()
       :m_vertices({}),
        m_sub_meshes({})
    {
    }
    GameMesh::~GameMesh()
    {
    }



    //Game Sub Mesh contains Indices of certain mesh.
    GameSubMesh::GameSubMesh():
        m_indices({})
    {
    }
    GameSubMesh::~GameSubMesh()
    {
    }


    //Game Materials
    GameMeterial::GameMeterial():
        m_meshes({})
    {
    }
    GameMeterial::~GameMeterial()
    {
    }
}