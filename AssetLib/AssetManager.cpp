#include "AssetManager.h"
#include "ObjLoaderHelper.h"
#include "FBXLoaderHelper.h"
#include "DX12RendererConfig.h"
#include <DX12Texture.h>


assetlib::AssetManager::AssetManager()
{
    Init();
}

assetlib::AssetManager::~AssetManager()
{
}

void assetlib::AssetManager::Init()
{
}

void assetlib::AssetManager::Update()
{
}

void assetlib::AssetManager::Destory()
{
}

void assetlib::AssetManager::LoadScene(const std::string & p_name)
{
    using namespace Renderer;

    std::shared_ptr<gameplay::GamesScene> l_scene = assetlib::FBXLoader::GetFbxLoader().LoadFBX(KEngineConstants::MODEL_ASSET_DIR + p_name);
    assert(l_scene && "Nullptr!");

    m_scenes[p_name] = l_scene;

    {
        // Define the geometry for a triangle.
        for (auto& l_mesh : l_scene->dummy_actor->m_meshes) 
        {
            LoadMesh(l_mesh);
            if (ENABLE_ACTOR_BOUNDING_BOX)
            {
                l_scene->dummy_actor->GenerateBoundingBox();
                LoadMesh(l_scene->dummy_actor->m_bounding_box_mesh);
            }
        }
    }

}

void assetlib::AssetManager::LoadMesh(gameplay::GameMesh * l_mesh)
{
    using namespace Renderer;

    const uint64_t vertexBufferSize = static_cast<uint64_t>(l_mesh->GetVertices().size()) * sizeof(Vertex);

    //Vertex Job
    TransferJob l_vertex_upload_job = {};
    l_vertex_upload_job.data = l_mesh->GetVertices().data();
    l_vertex_upload_job.data_size = vertexBufferSize;
    l_vertex_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX;
    l_vertex_upload_job.vertex_count = l_mesh->GetVertexCount();

    DX12TransferManager::GetTransferManager().AddTransferJob(&l_vertex_upload_job, true);
    l_mesh->SetVertexOffsetInBuffer(l_vertex_upload_job.vertex_offset);

    //Index Job
    TransferJob l_index_upload_job = {};
    l_index_upload_job.data = l_mesh->GetIndices().data();
    l_index_upload_job.data_size = sizeof(uint32_t) * l_mesh->GetIndexCount();
    l_index_upload_job.type = TransferJob::JobType::UPLOAD_INDEX;
    l_index_upload_job.index_count = l_mesh->GetIndexCount();

    DX12TransferManager::GetTransferManager().AddTransferJob(&l_index_upload_job, true);
    l_mesh->SetIndexOffsetInBuffer(l_index_upload_job.index_offet);

    //Release system memory used by vertices and indices.
    //They are useless now since we don't readback or reuse it.
    //l_mesh->ReleaseMeshData();
}

void assetlib::AssetManager::LoadMesh(std::shared_ptr<gameplay::GameMesh> l_mesh)
{
    using namespace Renderer;

    const uint64_t vertexBufferSize = static_cast<uint64_t>(l_mesh->GetVertices().size()) * sizeof(Vertex);

    //Vertex Job
    TransferJob l_vertex_upload_job = {};
    l_vertex_upload_job.data = l_mesh->GetVertices().data();
    l_vertex_upload_job.data_size = vertexBufferSize;
    l_vertex_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX;
    l_vertex_upload_job.vertex_count = l_mesh->GetVertexCount();

    DX12TransferManager::GetTransferManager().AddTransferJob(&l_vertex_upload_job, true);
    l_mesh->SetVertexOffsetInBuffer(l_vertex_upload_job.vertex_offset);

    //Index Job
    TransferJob l_index_upload_job = {};
    l_index_upload_job.data = l_mesh->GetIndices().data();
    l_index_upload_job.data_size = sizeof(uint32_t) * l_mesh->GetIndexCount();
    l_index_upload_job.type = TransferJob::JobType::UPLOAD_INDEX;
    l_index_upload_job.index_count = l_mesh->GetIndexCount();

    DX12TransferManager::GetTransferManager().AddTransferJob(&l_index_upload_job, true);
    l_mesh->SetIndexOffsetInBuffer(l_index_upload_job.index_offet);

    //Release system memory used by vertices and indices.
    //They are useless now since we don't readback or reuse it.
    //l_mesh->ReleaseMeshData();
}


