#include "FBXLoaderHelper.h"
#include <fbxsdk/include/fbxsdk.h>
#include <KLogger.h>

static FbxManager* pManager;


static void InitFBXSDK() 
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    
    pManager = FbxManager::Create();
    if (!pManager)
    {
        KLOG_CRITICAL("Failed to Init FBX Manager");
        assert(0);
    }
    else KLOG_INFO("Autodesk FBX SDK version {0}\n", pManager->GetVersion());

    //Create an IOSettings object. This object holds all import/export settings.
    FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
    pManager->SetIOSettings(ios);

    //Load plugins from the executable directory (optional)
    FbxString lPath = FbxGetApplicationDirectory();
    pManager->LoadPluginsDirectory(lPath.Buffer());
}

std::unique_ptr<gameplay::GamesScene> assetlib::LoadFBX(const std::string & p_file_name)
{

    InitFBXSDK();

    auto l_scene = std::unique_ptr<gameplay::GamesScene>(new gameplay::GamesScene(p_file_name));

    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor, lSDKMinor, lSDKRevision;
    //int lFileFormat = -1;
    int i, lAnimStackCount;
    bool lStatus;
    char lPassword[1024];

    // Get the file version number generate by the FBX SDK.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pManager, "");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(p_file_name.c_str(), -1, pManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if (!lImportStatus)
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        KLOG_CRITICAL("Call to FbxImporter::Initialize() failed.\n");
        KLOG_CRITICAL("Error returned: {0}", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            KLOG_CRITICAL("FBX file format version for this FBX SDK is {0:d}.{0:d}.{0:d}\n", lSDKMajor, lSDKMinor, lSDKRevision);
            KLOG_CRITICAL("FBX file format version for file '%s' is {0:d}.{0:d}.{0:d}\n\n", p_file_name.c_str(), lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    KLOG_INFO("FBX file format version for this FBX SDK is {0:d}.{1:d}.{2:d}\n", lSDKMajor, lSDKMinor, lSDKRevision);


    return l_scene;
}
