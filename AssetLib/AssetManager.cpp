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

    std::shared_ptr<gameplay::GamesScene> l_scene = assetlib::FBXLoader::GetFbxLoader().LoadFBX(KEngineConstants::MODEL_ASSET_DIR + p_name);
    assert(l_scene && "Nullptr!");

    m_scenes[p_name] = l_scene;

    {
        // Define the geometry for a triangle.
        for (auto& l_mesh : l_scene->dummy_actor->m_meshes) 
        {
            LoadMesh(l_mesh);
        }
        DX12TransferManager::GetTransferManager().PrepareToRender();
    }

}

void assetlib::AssetManager::LoadMesh(gameplay::GameMesh * l_mesh)
{
    using namespace Renderer;

    const uint64_t vertexBufferSize = static_cast<uint64_t>(l_mesh->m_vertices.size()) * sizeof(float);

    //Vertex Job
    TransferJob l_vertex_upload_job = {};
    l_vertex_upload_job.data = l_mesh->m_vertices.data();
    l_vertex_upload_job.data_size = vertexBufferSize;
    l_vertex_upload_job.type = TransferJob::JobType::UPLOAD_VERTEX;
    l_vertex_upload_job.vertex_count = l_mesh->m_vertices.size() / KEngineConstants::FLOAT_COUNT_PER_VERTEX;

    DX12TransferManager::GetTransferManager().AddTransferJob(&l_vertex_upload_job, true);
    l_mesh->m_vertex_offset = l_vertex_upload_job.vertex_offset;

    //Index Job
    TransferJob l_index_upload_job = {};
    l_index_upload_job.data = l_mesh->m_indices.data();
    l_index_upload_job.data_size = sizeof(uint32_t) * l_mesh->m_indices.size();
    l_index_upload_job.type = TransferJob::JobType::UPLOAD_INDEX;
    l_index_upload_job.index_count = l_mesh->m_indices.size();

    DX12TransferManager::GetTransferManager().AddTransferJob(&l_index_upload_job, true);
    l_mesh->m_index_offset = l_index_upload_job.index_offet;

    //Release system memory used by vertices and indices.
    //They are useless now since we don't readback or reuse it.
    l_mesh->ReleaseMeshData();
}


