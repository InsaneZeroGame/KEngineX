#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "ObjLoaderHelper.h"
#include <cassert>
#include <EngineConfig.h>



std::unique_ptr<gameplay::GamesScene> assetlib::LoadObj(const std::string & p_file_name)
{

    using namespace gameplay;


    auto l_scene = std::unique_ptr< GamesScene>(new  GamesScene());
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

        printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
        printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
        printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
        printf("# of materials = %d\n", (int)materials.size());
        printf("# of shapes    = %d\n", (int)shapes.size());

        auto l_material = std::shared_ptr< GameMeterial>(new GameMeterial());


        {
            //Map Shapes to Engine GameMesh
            std::vector<float> vertex_buffer;
            std::vector<uint32_t> index_buffer;
            for (size_t s = 0; s < shapes.size(); s++)
            {
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
            l_material->m_meshes.push_back(GameMesh(vertex_buffer, index_buffer));

        }
        l_scene->dummy_actor->AddMaterial(l_material);


    }
    return l_scene;
}
