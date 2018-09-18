#include "FBXLoaderHelper.h"
#include <fbxsdk/include/fbxsdk.h>
#include <KLogger.h>

static FbxManager* pManager;
static uint64_t vertex_offset = 0;


void DisplayPolygons(FbxMesh* pMesh, gameplay::GamesScene* p_game_scene)
{
    int i, j, lPolygonCount = pMesh->GetPolygonCount();
    FbxVector4* lControlPoints = pMesh->GetControlPoints();

    //We map Fbx::mesh -> Gameplay::mesh
    //1.All vertices in Fbx::mesh -> Gameplay::mesh vertices.
    //2.Since all veritces is in one vertex buffer,vertex index in Fbx::mesh is added an index offset per Fbx::mesh(Gameplay::sbumesh).


    //Add by Ang to map Fbx::mesh -> Gameplay::submesh.
    std::vector<uint32_t> indices = {};

    for (i = 0; i < lPolygonCount; i++)
    {
        int lPolygonSize = pMesh->GetPolygonSize(i);
        for (j = 0; j < lPolygonSize; j++)
        {
            int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
            indices.push_back(lControlPointIndex + vertex_offset);
        } // for polygonSize
    } // for polygonCount
    gameplay::GameSubMesh l_submesh(indices);

    p_game_scene->dummy_actor->m_mesh->m_sub_meshes.push_back(l_submesh);
}


static void DisplayControlsPoints(FbxMesh* pMesh, gameplay::GamesScene* p_game_scene)
{
    int i, lControlPointsCount = pMesh->GetControlPointsCount();
    vertex_offset += lControlPointsCount;
    FbxVector4* lControlPoints = pMesh->GetControlPoints();


    for (i = 0; i < lControlPointsCount; i++)
    {
        p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(lControlPoints[i].Buffer()[0]);
        p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(lControlPoints[i].Buffer()[1]);
        p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(lControlPoints[i].Buffer()[2]);

        for (int j = 0; j < pMesh->GetElementNormalCount(); j++)
        {
            FbxGeometryElementNormal* leNormals = pMesh->GetElementNormal(j);
            if (leNormals->GetMappingMode() == FbxGeometryElement::eByControlPoint)
            {
                if (leNormals->GetReferenceMode() == FbxGeometryElement::eDirect)
                {
                    p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(leNormals->GetDirectArray().GetAt(i).Buffer()[0]);
                    p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(leNormals->GetDirectArray().GetAt(i).Buffer()[1]);
                    p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(leNormals->GetDirectArray().GetAt(i).Buffer()[2]);
                    p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(1.0f);
                    p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(0.0f);
                    p_game_scene->dummy_actor->m_mesh->m_vertices.push_back(0.0f);
                }
            }
        }
    }
}


void DisplayMesh(FbxNode* pNode, gameplay::GamesScene* p_game_scene)
{
    FbxMesh* lMesh = (FbxMesh*)pNode->GetNodeAttribute();

    //DisplayMetaDataConnections(lMesh);
    DisplayPolygons(lMesh, p_game_scene);
    DisplayControlsPoints(lMesh, p_game_scene);
    //DisplayMaterialMapping(lMesh);
    //DisplayMaterial(lMesh);
    //DisplayTexture(lMesh);
    //DisplayMaterialConnections(lMesh);
    //DisplayLink(lMesh);
    //DisplayShape(lMesh);
    //
    //DisplayCache(lMesh);
}


void DisplayContent(FbxNode* pNode,gameplay::GamesScene* p_game_scene)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

    if (pNode->GetNodeAttribute() == NULL)
    {
        KLOG_INFO("NULL Node Attribute");
    }
    else
    {
        lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

        switch (lAttributeType)
        {
        default:
            break;
        case FbxNodeAttribute::eMarker:
        case FbxNodeAttribute::eSkeleton:
        case FbxNodeAttribute::eNurbs:
        case FbxNodeAttribute::ePatch:
        case FbxNodeAttribute::eCamera:
        case FbxNodeAttribute::eLight:
        case FbxNodeAttribute::eLODGroup:
            KLOG_CRITICAL("Not Support Yet!");
            break;
        case FbxNodeAttribute::eMesh:
            KLOG_INFO("Sub Mesh");
            DisplayMesh(pNode, p_game_scene);
            break;
        }
        //DisplayUserProperties(pNode);
        //DisplayTarget(pNode);
        //DisplayPivotsAndLimits(pNode);
        //DisplayTransformPropagation(pNode);
        //DisplayGeometricTransform(pNode);

        for (i = 0; i < pNode->GetChildCount(); i++)
        {
            DisplayContent(pNode->GetChild(i), p_game_scene);
        }
    }
}


static void DisplayContent(FbxScene* pScene,gameplay::GamesScene* p_game_scene)
{
    int i;
    FbxNode* lNode = pScene->GetRootNode();

    if (lNode)
    {
        for (i = 0; i < lNode->GetChildCount(); i++)
        {
            DisplayContent(lNode->GetChild(i), p_game_scene);
        }
    }
}


static void InitFBXSDK() 
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if (!pManager)
    {
        KLOG_INFO("Error: Unable to create FBX Manager! ");
        exit(1);
    }
    else KLOG_INFO("Autodesk FBX SDK version {0} ", pManager->GetVersion());

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

    auto l_scene = new gameplay::GamesScene(p_file_name);

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
        KLOG_CRITICAL("Call to FbxImporter::Initialize() failed. ");
        KLOG_CRITICAL("Error returned: {0}", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            KLOG_CRITICAL("FBX file format version for this FBX SDK is {0:d}.{0:d}.{0:d} ", lSDKMajor, lSDKMinor, lSDKRevision);
            KLOG_CRITICAL("FBX file format version for file '%s' is {0:d}.{0:d}.{0:d}  ", p_file_name.c_str(), lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    if (lImporter->IsFBX())
    {
        KLOG_INFO("FBX file format version for file {0} is {1:d}.{2:d}.{3:d}  ", p_file_name, lFileMajor, lFileMinor, lFileRevision);

        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.

        KLOG_INFO("Animation Stack Information ");

        lAnimStackCount = lImporter->GetAnimStackCount();

        KLOG_INFO("    Number of Animation Stacks: {0:d} ", lAnimStackCount);
        KLOG_INFO("    Current Animation Stack: \"{0}\" ", lImporter->GetActiveAnimStackName().Buffer());
        KLOG_INFO(" ");

        for (i = 0; i < lAnimStackCount; i++)
        {
            FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

            KLOG_INFO("    Animation Stack {0:d} ", i);
            KLOG_INFO("         Name: \"%s\" ", lTakeInfo->mName.Buffer());
            KLOG_INFO("         Description: \"{0}\" ", lTakeInfo->mDescription.Buffer());

            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            KLOG_INFO("         Import Name: \"{0}\" ", lTakeInfo->mImportName.Buffer());

            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            KLOG_INFO("         Import State: {0} ", lTakeInfo->mSelect ? "true" : "false");
            KLOG_INFO(" ");
        }
    }


    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    auto pScene = FbxScene::Create(pManager, "My Scene");
    if (!pScene)
    {
        KLOG_CRITICAL("Error: Unable to create FBX scene! ");
        assert(0);
    }
    lStatus = lImporter->Import(pScene);
    DisplayContent(pScene,l_scene);
    pScene->Destroy();
    lImporter->Destroy();
    using namespace gameplay;

    //Setup Main Camera
    Vector3 eye = Vector3(3.0f, 3.0f, 3.0f);
    Vector3 at = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

    l_scene->m_main_camera.SetEyeAtUp(eye, at, up);
    l_scene->m_main_camera.SetPerspectiveMatrix(45.0f * 3.1415f / 180.0f, 600.0f / 800.0f, 0.5f, 6.0f);
    l_scene->m_main_camera.Update();

    //Setup Shadow Camera
    eye = Vector3(-0.1f, 0.1f, 0.1f);
    at = Vector3(0.0f, 0.0f, 0.0f);
    up = Vector3(0.0f, 1.0f, 0.0f);

    l_scene->m_shadow_camera.SetEyeAtUp(eye, at, up);
    l_scene->m_shadow_camera.SetPerspectiveMatrix(45.0f * 3.1415f / 180.0f, 600.0f / 800.0f, 0.5f, 15.0f);
    l_scene->m_shadow_camera.Update();

    return std::unique_ptr<gameplay::GamesScene>(l_scene);
}
