#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "ObjLoaderHelper.h"
#include <cassert>
#include <EngineConfig.h>



std::unique_ptr<gameplay::GamesScene> assetlib::LoadObj(const std::string & p_file_name)
{

    using namespace gameplay;


    auto l_scene = std::unique_ptr< GamesScene>(new  GamesScene(p_file_name));
    {
        std::vector<tinyobj::material_t> materials;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, (KEngineConstants::MODEL_ASSET_DIR + p_file_name).c_str(),
            KEngineConstants::MODEL_ASSET_DIR.c_str());
        if (!err.empty())
        {
            assert(0);
        }


        if (!ret)
        {
            assert(0);
        }

        //printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
        //printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
        //printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
        //printf("# of materials = %d\n", (int)materials.size());
        //printf("# of shapes    = %d\n", (int)shapes.size());

        auto l_material = std::shared_ptr< GameMeterial>(new GameMeterial());


        {
            //Map Shapes to Engine GameMesh
            std::vector<float> vertex_buffer;
            GameMesh l_mesh;
            for (size_t s = 0; s < shapes.size(); s++)
            {
                //Pershape mapped to GameSubMesh
                std::vector<uint32_t> index_buffer;
                for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++)
                {
                    tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                    tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                    tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;

                    index_buffer.push_back(f0);
                    index_buffer.push_back(f1);
                    index_buffer.push_back(f2);

                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);
                }
                GameSubMesh l_sub_mesh(index_buffer);
                //TinyObjLoader support a per face material,we only use face 0 material.
                auto material_id = shapes[s].mesh.material_ids[0];
                l_sub_mesh.m_diffuse[0] = materials[material_id].diffuse[0];
                l_sub_mesh.m_diffuse[1] = materials[material_id].diffuse[1];
                l_sub_mesh.m_diffuse[2] = materials[material_id].diffuse[2];
                l_sub_mesh.m_diffuse[3] = 1.0f;

                l_mesh.m_sub_meshes.push_back(l_sub_mesh);
            }
            uint32_t i = 0;
            for (auto& vertex : attrib.vertices)
            {
                vertex_buffer.push_back(vertex);
                if (i % 3 == 2)
                {
                    vertex_buffer.push_back(1.0f);
                    vertex_buffer.push_back(1.0f);
                    vertex_buffer.push_back(1.0f);
                    vertex_buffer.push_back(1.0f);
                }
                ++i;
            }
            l_mesh.m_vertices = vertex_buffer;
            l_material->m_meshes.push_back(l_mesh);

        }
        l_scene->dummy_actor->AddMaterial(l_material);
    }


    //Setup Main Camera
    Vector3 eye = Vector3(8.0f, 5.0f, 5.0f);
    Vector3 at  = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 up  = Vector3(0.0f,1.0f,0.0f);

    l_scene->m_main_camera.SetEyeAtUp(eye, at, up);
    l_scene->m_main_camera.SetPerspectiveMatrix(45.0f * 3.1415f / 180.0f,600.0f/800.0f,0.1f, 15.0f);
    l_scene->m_main_camera.Update();

    //Setup Shadow Camera
    eye = Vector3(-8.0f, 5.0f, 5.0f);
    at = Vector3(0.0f, 0.0f, 0.0f);
    up = Vector3(0.0f, 1.0f, 0.0f);

    l_scene->m_shadow_camera.SetEyeAtUp(eye, at, up);
    l_scene->m_shadow_camera.SetPerspectiveMatrix(45.0f * 3.1415f / 180.0f, 600.0f / 800.0f, 0.1f, 15.0f);
    l_scene->m_shadow_camera.Update();

    return l_scene;
}
