#include "AssetManager.h"
#include "ObjLoaderHelper.h"

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


    LoadObj("cube.obj");

}

void assetlib::AssetManager::Update()
{
}

void assetlib::AssetManager::Destory()
{
}
