#include "AssetManager.h"
#include "ObjLoaderHelper.h"
#include "FBXLoaderHelper.h"
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

    //ToDo:Fbx Loader
    //// Change the following filename to a suitable filename value.
    //const char* lFilename = "D:\\Dev\\KEngineX\\Assets\\Models\\humanoid.fbx";
    //
    //// Initialize the SDK manager. This object handles all our memory management.
    //FbxManager* lSdkManager = FbxManager::Create();
    //
    //// Create the IO settings object.
    //FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    //lSdkManager->SetIOSettings(ios);
    //
    //// Create an importer using the SDK manager.
    //FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
    //
    //// Use the first argument as the filename for the importer.
    //if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
    //    printf("Call to FbxImporter::Initialize() failed.\n");
    //    printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
    //    exit(-1);
    //}
    //
    //// Create a new scene so that it can be populated by the imported file.
    //FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
    //
    //// Import the contents of the file into the scene.
    //lImporter->Import(lScene);
    //
    //// The file is imported; so get rid of the importer.
    //lImporter->Destroy();
    //
    //// Print the nodes of the scene and their attributes recursively.
    //// Note that we are not printing the root node because it should
    //// not contain any attributes.
    //FbxNode* lRootNode = lScene->GetRootNode();
    //if (lRootNode) {
    //    for (int i = 0; i < lRootNode->GetChildCount(); i++)
    //        ;
    //}
    //// Destroy the SDK manager and all the other objects it was handling.
    //lSdkManager->Destroy();



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

    std::shared_ptr<gameplay::GamesScene> l_scene = LoadFBX(KEngineConstants::MODEL_ASSET_DIR + p_name);
    assert(l_scene && "Nullptr!");

    m_scenes[p_name] = l_scene;

    {
        struct Vertex
        {
            float position[3];
            float color[4];
            float texture_coord[2];
        };
        // Define the geometry for a triangle.

        const uint64_t vertexBufferSize = static_cast<uint64_t>(l_scene->dummy_actor->m_mesh->m_vertices.size()) * sizeof(float);


        TransferJob l_vertex_upload_job = {};
        l_vertex_upload_job.data = l_scene->dummy_actor->m_mesh->m_vertices.data();
        l_vertex_upload_job.data_size = vertexBufferSize;
        l_vertex_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX_BUFFER;

        DX12TransferManager::GetTransferManager().AddTransferJob(&l_vertex_upload_job, true);
        // Initialize the vertex buffer view.
        l_scene->dummy_actor->m_mesh->m_vertex_buffer_desc.BufferLocation = l_vertex_upload_job.gpu_va_address;
        l_scene->dummy_actor->m_mesh->m_vertex_buffer_desc.StrideInBytes = sizeof(Vertex);
        l_scene->dummy_actor->m_mesh->m_vertex_buffer_desc.SizeInBytes = static_cast<uint32_t>(vertexBufferSize);

        
        for (auto & submesh : l_scene->dummy_actor->m_mesh->m_sub_meshes)
        {
            TransferJob l_submesh_upload_job = {};
            l_submesh_upload_job.data = submesh.m_indices.data();
            l_submesh_upload_job.data_size = sizeof(uint32_t) * submesh.m_indices.size();
            l_submesh_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX_BUFFER;

            DX12TransferManager::GetTransferManager().AddTransferJob(&l_submesh_upload_job, true);
            // Initialize the Index buffer view.
            submesh.m_index_buffer_desc.BufferLocation = l_submesh_upload_job.gpu_va_address;
            submesh.m_index_buffer_desc.StrideInBytes = sizeof(uint32_t);
            submesh.m_index_buffer_desc.SizeInBytes = static_cast<uint32_t>(l_submesh_upload_job.data_size);
        }

        //Release system memory used by vertices and indices.
        //They are useless now since we don't readback or reuse it.
        l_scene->dummy_actor->m_mesh->ReleaseMeshData();

        DX12TransferManager::GetTransferManager().PrepareToRender();
    }

}


