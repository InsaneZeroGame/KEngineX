#include "FBXLoaderHelper.h"
#include <fbxsdk.h>
#include <KLogger.h>
#include "EngineConfig.h"
#include "AssetManager.h"
#define MAT_HEADER_LENGTH 200

static FbxManager* pManager;



namespace assetlib
{

    FBXLoader::FBXLoader()
    {

        InitFBXSDK();
    }

    FBXLoader::~FBXLoader()
    {
    }

    std::unique_ptr<gameplay::GamesScene> FBXLoader::LoadFBX(const std::string & p_file_name)
    {
        m_scene = new gameplay::GamesScene(p_file_name);

        int lFileMajor, lFileMinor, lFileRevision;
        int lSDKMajor, lSDKMinor, lSDKRevision;
        //int lFileFormat = -1;
        int i, lAnimStackCount;
        bool lStatus;
        //char lPassword[1024];

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
        DisplayContent(pScene, m_scene);
        pScene->Destroy();
        lImporter->Destroy();
        using namespace gameplay;

        //Setup Main Camera
        Vector3 eye = Vector3(10.0f, 10.0f, 10.0f);
        Vector3 at = Vector3(0.0f, 0.0f, 0.0f);
        Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

        m_scene->m_main_camera.SetEyeAtUp(eye, at, up);
        m_scene->m_main_camera.SetPerspectiveMatrix(45.0f * 3.1415f / 180.0f, 600.0f / 800.0f, 8.0f, 25.0f);
        m_scene->m_main_camera.Update();

        //Setup Shadow Camera
        eye = Vector3(-10.0f, 10.0f, 10.0f);
        at = Vector3(0.0f, 0.0f, 0.0f);
        up = Vector3(0.0f, 1.0f, 0.0f);

        m_scene->m_shadow_camera.SetEyeAtUp(eye, at, up);
        m_scene->m_shadow_camera.SetPerspectiveMatrix(45.0f * 3.1415f / 180.0f, 600.0f / 800.0f, 8.0f, 25.0f);
        m_scene->m_shadow_camera.Update();

        return std::unique_ptr<gameplay::GamesScene>(m_scene);
    }
    void FBXLoader::DisplayMesh(FbxNode* pNode, gameplay::GamesScene* p_game_scene)
    {
        DisplayString("Mesh Name: ", (char *)pNode->GetName());

        FbxMesh* lMesh = (FbxMesh*)pNode->GetNodeAttribute();
        //Add new mesh.
        auto l_game_mesh = new gameplay::GameMesh(std::string((char *)pNode->GetName()));
        FbxAMatrix matrixGeo;
        matrixGeo.SetIdentity();
        const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
        const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
        const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
        matrixGeo.SetT(lT);
        matrixGeo.SetR(lR);
        matrixGeo.SetS(lS);
        FbxAMatrix globalMatrix = pNode->EvaluateLocalTransform();
        FbxAMatrix matrix = globalMatrix * matrixGeo;

        //DisplayControlsPoints(lMesh, l_game_mesh, &matrix);
        DisplayPolygons(lMesh, l_game_mesh,&matrix);

        //DisplayMaterialMapping(lMesh);
        //DisplayMaterial(lMesh);
        DisplayTexture(lMesh);
        //DisplayMaterialConnections(lMesh);
        p_game_scene->dummy_actor->m_meshes.push_back(l_game_mesh);
    }
    void FBXLoader::DisplayContent(FbxNode* pNode, gameplay::GamesScene* p_game_scene)
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
    void FBXLoader::DisplayContent(FbxScene* pScene, gameplay::GamesScene* p_game_scene)
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
    void FBXLoader::InitFBXSDK()
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
    void FBXLoader::DisplayPolygons(FbxMesh* pMesh, gameplay::GameMesh* p_game_mesh, FbxAMatrix* p_transform_matrix)
    {
        int i, j, lPolygonCount = pMesh->GetPolygonCount();
        int lControlPointsCount = pMesh->GetControlPointsCount();
        FbxVector4* lControlPoints = pMesh->GetControlPoints();

        //Get vertex offset from of current vertex buffer.
        //p_game_mesh->m_vertex_offset = assetlib::AssetManager::GetAssertManager().m_vertex_id_offset;
        //Get index offset from of current index buffer.
        //p_game_mesh->m_index_offset = assetlib::AssetManager::GetAssertManager().m_index_id_offset;

        using namespace KEngineConstants;

        p_game_mesh->m_vertices.resize(lControlPointsCount * FLOAT_COUNT_PER_VERTEX);

        int vertexId = 0;
        for (i = 0; i < lPolygonCount; i++)
        {
            int lPolygonSize = pMesh->GetPolygonSize(i);
            for (j = 0; j < lPolygonSize; j++)
            {
                //Pos
                int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
                auto transformed_control_points = p_transform_matrix->MultT(lControlPoints[lControlPointIndex]);
                p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 0] = static_cast<float>(transformed_control_points.Buffer()[0]);
                p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 1] = static_cast<float>(transformed_control_points.Buffer()[1]);
                p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 2] = static_cast<float>(transformed_control_points.Buffer()[2]);
                
                //Normal
                for (auto l = 0; l < pMesh->GetElementNormalCount(); ++l)
                {
                    FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);
                
                    if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                    {
                        switch (leNormal->GetReferenceMode())
                        {
                        case FbxGeometryElement::eDirect:
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 3] = static_cast<float>(leNormal->GetDirectArray().GetAt(vertexId).Buffer()[0]);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 4] = static_cast<float>(leNormal->GetDirectArray().GetAt(vertexId).Buffer()[1]);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 5] = static_cast<float>(leNormal->GetDirectArray().GetAt(vertexId).Buffer()[2]);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 6] = 1.0f;
                            break;
                        case FbxGeometryElement::eIndexToDirect:
                        {
                            int id = leNormal->GetIndexArray().GetAt(vertexId);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 3] = static_cast<float>(leNormal->GetDirectArray().GetAt(id).Buffer()[0]);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 4] = static_cast<float>(leNormal->GetDirectArray().GetAt(id).Buffer()[1]);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 5] = static_cast<float>(leNormal->GetDirectArray().GetAt(id).Buffer()[2]);
                            p_game_mesh->m_vertices[vertexId * FLOAT_COUNT_PER_VERTEX + 6] = 1.0f;
                        }
                        break;
                        default:
                            break; // other reference modes not shown here!
                        }
                    }
                    else if (leNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                    {
                        if (leNormal->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 3] = static_cast<float>(leNormal->GetDirectArray().GetAt(lControlPointIndex).Buffer()[0]);
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 4] = static_cast<float>(leNormal->GetDirectArray().GetAt(lControlPointIndex).Buffer()[1]);
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 5] = static_cast<float>(leNormal->GetDirectArray().GetAt(lControlPointIndex).Buffer()[2]);
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 6] = 1.0f;
                        }
                    }
                
                }

                //UV Coords
                for (auto l = 0; l < pMesh->GetElementUVCount(); ++l)
                {
                    FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
                
                    switch (leUV->GetMappingMode())
                    {
                    default:
                        break;
                    case FbxGeometryElement::eByControlPoint:
                        switch (leUV->GetReferenceMode())
                        {
                        case FbxGeometryElement::eDirect:
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 7] = leUV->GetDirectArray().GetAt(lControlPointIndex).Buffer()[0];
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 8] = leUV->GetDirectArray().GetAt(lControlPointIndex).Buffer()[1];
                            break;
                        case FbxGeometryElement::eIndexToDirect:
                        {
                            int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 7] = leUV->GetDirectArray().GetAt(id).Buffer()[0];
                            p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 8] = leUV->GetDirectArray().GetAt(id).Buffer()[1];

                        }
                        break;
                        default:
                            break; // other reference modes not shown here!
                        }
                        break;
                
                    case FbxGeometryElement::eByPolygonVertex:
                    {
                        int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);
                        switch (leUV->GetReferenceMode())
                        {
                        case FbxGeometryElement::eDirect:
                            assert(0);
                            break;
                        case FbxGeometryElement::eIndexToDirect:
                        {
                            if (vertexId < leUV->GetIndexArray().GetCount())
                            {
                                FbxVector2 lUVValue;

                                //the UV index depends on the reference mode
                                //int lUVIndex = leUV->GetIndexArray().GetAt(vertexId);
                                lUVValue = leUV->GetDirectArray().GetAt(lTextureUVIndex);
                                p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 7] = lUVValue.Buffer()[0];
                                p_game_mesh->m_vertices[lControlPointIndex * FLOAT_COUNT_PER_VERTEX + 8] = lUVValue.Buffer()[1];
                                vertexId++;

                            }
                           }
                        break;
                        default:
                            assert(0);
                            break; // other reference modes not shown here!
                        }
                    }
                    break;
                
                    case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                    case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                    case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                        break;
                    }
                }

                p_game_mesh->m_indices.push_back(lControlPointIndex);
            } // for polygonSize
        } // for polygonCount
        p_game_mesh->m_index_count = p_game_mesh->m_indices.size();
        //assetlib::AssetManager::GetAssertManager().m_index_id_offset += p_game_mesh->m_indices.size();
        //assetlib::AssetManager::GetAssertManager().m_vertex_id_offset += lControlPointsCount;

    }

    void FBXLoader::DisplayTextureNames(FbxProperty &pProperty, FbxString& pConnectionString)
    {
        int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
        if (lLayeredTextureCount > 0)
        {
            for (int j = 0; j < lLayeredTextureCount; ++j)
            {
                FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
                int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
                pConnectionString += " Texture ";
    
                for (int k = 0; k < lNbTextures; ++k)
                {
                    //lConnectionString += k;
                    pConnectionString += "\"";
                    pConnectionString += (char*)lLayeredTexture->GetName();
                    pConnectionString += "\"";
                    pConnectionString += " ";
                }
                pConnectionString += "of ";
                pConnectionString += pProperty.GetName();
                pConnectionString += " on layer ";
                pConnectionString += j;
            }
            pConnectionString += " |";
        }
        else
        {
            //no layered texture simply get on the property
            int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();
    
            if (lNbTextures > 0)
            {
                pConnectionString += " Texture ";
                pConnectionString += " ";
    
                for (int j = 0; j < lNbTextures; ++j)
                {
                    FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                    if (lTexture)
                    {
                        pConnectionString += "\"";
                        pConnectionString += (char*)lTexture->GetName();
                        pConnectionString += "\"";
                        pConnectionString += " ";
                    }
                }
                pConnectionString += "of ";
                pConnectionString += pProperty.GetName();
                pConnectionString += " |";
            }
        }
    }
    void FBXLoader::DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char * header, int pMatId, int l)
    {
        if (!pMaterial)
            return;
    
        FbxString lConnectionString = "            Material %d -- ";
        //Show all the textures
    
        FbxProperty lProperty;
        //Diffuse Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //DiffuseFactor Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Emissive Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //EmissiveFactor Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissiveFactor);
        DisplayTextureNames(lProperty, lConnectionString);
    
    
        //Ambient Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //AmbientFactor Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Specular Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //SpecularFactor Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Shininess Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Bump Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sBump);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Normal Map Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Transparent Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparentColor);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //TransparencyFactor Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Reflection Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflection);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //ReflectionFactor Textures
        lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
        DisplayTextureNames(lProperty, lConnectionString);
    
        //Update header with material info
        bool lStringOverflow = (lConnectionString.GetLen() + 10 >= MAT_HEADER_LENGTH); // allow for string length and some padding for "%d"
        if (lStringOverflow)
        {
            // Truncate string!
            lConnectionString = lConnectionString.Left(MAT_HEADER_LENGTH - 10);
            lConnectionString = lConnectionString + "...";
        }
        FBXSDK_sprintf(header, MAT_HEADER_LENGTH, lConnectionString.Buffer(), pMatId, l);
        KLOG_CRITICAL(header);
    }
    void FBXLoader::DisplayMaterialConnections(FbxMesh* pMesh)
    {
        int i, l, lPolygonCount = pMesh->GetPolygonCount();
    
        char header[MAT_HEADER_LENGTH];
    
        KLOG_CRITICAL("    Polygons Material Connections");
    
        //check whether the material maps with only one mesh
        bool lIsAllSame = true;
        for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
        {
    
            FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
            if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eByPolygon)
            {
                lIsAllSame = false;
                break;
            }
        }
    
        //For eAllSame mapping type, just out the material and texture mapping info once
        if (lIsAllSame)
        {
            for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
            {
    
                FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
                if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame)
                {
                    FbxSurfaceMaterial* lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(0));
                    int lMatId = lMaterialElement->GetIndexArray().GetAt(0);
                    if (lMatId >= 0)
                    {
                        KLOG_INFO("        All polygons share the same material in mesh : {0:d}", l);
                        DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
                    }
                }
            }
    
            //no material
            if (l == 0)
                KLOG_CRITICAL("        no material applied");
        }
    
        //For eByPolygon mapping type, just out the material and texture mapping info once
        else
        {
            for (i = 0; i < lPolygonCount; i++)
            {
                KLOG_INFO("        Polygon {0:d}", i);
    
                for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
                {
    
                    FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
                    FbxSurfaceMaterial* lMaterial = NULL;
                    int lMatId = -1;
                    lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(i));
                    lMatId = lMaterialElement->GetIndexArray().GetAt(i);
    
                    if (lMatId >= 0)
                    {
                        DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
                    }
                }
            }
        }
    }
    void FBXLoader::DisplayMaterialMapping(FbxMesh* pMesh)
    {
        const char* lMappingTypes[] = { "None", "By Control Point", "By Polygon Vertex", "By Polygon", "By Edge", "All Same" };
        const char* lReferenceMode[] = { "Direct", "Index", "Index to Direct" };
    
        int lMtrlCount = 0;
        FbxNode* lNode = NULL;
        if (pMesh) {
            lNode = pMesh->GetNode();
            if (lNode)
                lMtrlCount = lNode->GetMaterialCount();
        }
    
        for (int l = 0; l < pMesh->GetElementMaterialCount(); l++)
        {
            FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
            if (leMat)
            {
                //char header[100];
                //KLOG_INFO(header, 100, "    Material Element {0:d}: ", l);
                //KLOG_INFO(header);
    
    
                KLOG_INFO("           Mapping: {0}", lMappingTypes[leMat->GetMappingMode()]);
                KLOG_INFO("           ReferenceMode: {0}", lReferenceMode[leMat->GetReferenceMode()]);
    
                int lMaterialCount = 0;
                FbxString lString;
    
                if (leMat->GetReferenceMode() == FbxGeometryElement::eDirect ||
                    leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                {
                    lMaterialCount = lMtrlCount;
                }
    
                if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex ||
                    leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                {
                    int i;
    
                    lString = "           Indices: ";
    
                    int lIndexArrayCount = leMat->GetIndexArray().GetCount();
                    for (i = 0; i < lIndexArrayCount; i++)
                    {
                        lString += leMat->GetIndexArray().GetAt(i);
    
                        if (i < lIndexArrayCount - 1)
                        {
                            lString += ", ";
                        }
                    }
    
                    lString += "\n";
    
                    KLOG_INFO("{0}",lString.Buffer());
                }
            }
        }
    }
    void FBXLoader::DisplayMaterial(FbxGeometry* pGeometry)
    {
        int lMaterialCount = 0;
        FbxNode* lNode = NULL;
        if (pGeometry) {
            lNode = pGeometry->GetNode();
            if (lNode)
                lMaterialCount = lNode->GetMaterialCount();
        }
    
        if (lMaterialCount > 0)
        {
            FbxPropertyT<FbxDouble3> lKFbxDouble3;
            FbxPropertyT<FbxDouble> lKFbxDouble1;
            FbxColor theColor;
    
            for (int lCount = 0; lCount < lMaterialCount; lCount++)
            {
                DisplayInt("        Material ", lCount);
    
                FbxSurfaceMaterial *lMaterial = lNode->GetMaterial(lCount);
    
                DisplayString("            Name: \"", (char *)lMaterial->GetName(), "\"");
    
                //Get the implementation to see if it's a hardware shader.
                const FbxImplementation* lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_HLSL);
                FbxString lImplemenationType = "HLSL";
                if (!lImplementation)
                {
                    lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_CGFX);
                    lImplemenationType = "CGFX";
                }
                if (lImplementation)
                {
                    //Now we have a hardware shader, let's read it
                    KLOG_INFO("            Hardware Shader Type: {0}", lImplemenationType.Buffer());
                    const FbxBindingTable* lRootTable = lImplementation->GetRootTable();
                    FbxString lFileName = lRootTable->DescAbsoluteURL.Get();
                    FbxString lTechniqueName = lRootTable->DescTAG.Get();
    
    
                    const FbxBindingTable* lTable = lImplementation->GetRootTable();
                    size_t lEntryNum = lTable->GetEntryCount();
    
                    for (int i = 0; i < (int)lEntryNum; ++i)
                    {
                        const FbxBindingTableEntry& lEntry = lTable->GetEntry(i);
                        const char* lEntrySrcType = lEntry.GetEntryType(true);
                        FbxProperty lFbxProp;
    
    
                        FbxString lTest = lEntry.GetSource();
                        KLOG_INFO("            Entry: {0}", lTest.Buffer());
    
    
                        if (strcmp(FbxPropertyEntryView::sEntryType, lEntrySrcType) == 0)
                        {
                            lFbxProp = lMaterial->FindPropertyHierarchical(lEntry.GetSource());
                            if (!lFbxProp.IsValid())
                            {
                                lFbxProp = lMaterial->RootProperty.FindHierarchical(lEntry.GetSource());
                            }
    
    
                        }
                        else if (strcmp(FbxConstantEntryView::sEntryType, lEntrySrcType) == 0)
                        {
                            lFbxProp = lImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
                        }
                        if (lFbxProp.IsValid())
                        {
                            if (lFbxProp.GetSrcObjectCount<FbxTexture>() > 0)
                            {
                                //do what you want with the textures
                                for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
                                {
                                    FbxFileTexture *lTex = lFbxProp.GetSrcObject<FbxFileTexture>(j);
                                    KLOG_INFO("           File Texture: {0}", lTex->GetFileName());
                                }
                                for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
                                {
                                    FbxLayeredTexture *lTex = lFbxProp.GetSrcObject<FbxLayeredTexture>(j);
                                    KLOG_INFO("        Layered Texture: {0}", lTex->GetName());
                                }
                                for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
                                {
                                    FbxProceduralTexture *lTex = lFbxProp.GetSrcObject<FbxProceduralTexture>(j);
                                    KLOG_INFO("     Procedural Texture: {0}", lTex->GetName());
                                }
                            }
                            else
                            {
                                FbxDataType lFbxType = lFbxProp.GetPropertyDataType();
                                FbxString blah = lFbxType.GetName();
                                if (FbxBoolDT == lFbxType)
                                {
                                    DisplayBool("                Bool: ", lFbxProp.Get<FbxBool>());
                                }
                                else if (FbxIntDT == lFbxType || FbxEnumDT == lFbxType)
                                {
                                    DisplayInt("                Int: ", lFbxProp.Get<FbxInt>());
                                }
                                else if (FbxFloatDT == lFbxType)
                                {
                                    DisplayDouble("                Float: ", lFbxProp.Get<FbxFloat>());
    
                                }
                                else if (FbxDoubleDT == lFbxType)
                                {
                                    DisplayDouble("                Double: ", lFbxProp.Get<FbxDouble>());
                                }
                                else if (FbxStringDT == lFbxType
                                    || FbxUrlDT == lFbxType
                                    || FbxXRefUrlDT == lFbxType)
                                {
                                    DisplayString("                String: ", lFbxProp.Get<FbxString>().Buffer());
                                }
                                else if (FbxDouble2DT == lFbxType)
                                {
                                    FbxDouble2 lDouble2 = lFbxProp.Get<FbxDouble2>();
                                    FbxVector2 lVect;
                                    lVect[0] = lDouble2[0];
                                    lVect[1] = lDouble2[1];
    
                                    Display2DVector("                2D vector: ", lVect);
                                }
                                else if (FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
                                {
                                    FbxDouble3 lDouble3 = lFbxProp.Get<FbxDouble3>();
    
    
                                    FbxVector4 lVect;
                                    lVect[0] = lDouble3[0];
                                    lVect[1] = lDouble3[1];
                                    lVect[2] = lDouble3[2];
                                    Display3DVector("                3D vector: ", lVect);
                                }
    
                                else if (FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
                                {
                                    FbxDouble4 lDouble4 = lFbxProp.Get<FbxDouble4>();
                                    FbxVector4 lVect;
                                    lVect[0] = lDouble4[0];
                                    lVect[1] = lDouble4[1];
                                    lVect[2] = lDouble4[2];
                                    lVect[3] = lDouble4[3];
                                    Display4DVector("                4D vector: ", lVect);
                                }
                                else if (FbxDouble4x4DT == lFbxType)
                                {
                                    FbxDouble4x4 lDouble44 = lFbxProp.Get<FbxDouble4x4>();
                                    for (int j = 0; j < 4; ++j)
                                    {
    
                                        FbxVector4 lVect;
                                        lVect[0] = lDouble44[j][0];
                                        lVect[1] = lDouble44[j][1];
                                        lVect[2] = lDouble44[j][2];
                                        lVect[3] = lDouble44[j][3];
                                        Display4DVector("                4x4D vector: ", lVect);
                                    }
    
                                }
                            }
    
                        }
                    }
                }
                else if (lMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
                {
                    // We found a Phong material.  Display its properties.
    
                    // Display the Ambient Color
                    lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Ambient;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Ambient: ", theColor);
    
                    // Display the Diffuse Color
                    lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Diffuse;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Diffuse: ", theColor);
    
                    // Display the Specular Color (unique to Phong materials)
                    lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Specular;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Specular: ", theColor);
    
                    // Display the Emissive Color
                    lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Emissive;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Emissive: ", theColor);
    
                    //Opacity is Transparency factor now
                    lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->TransparencyFactor;
                    DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
    
                    // Display the Shininess
                    lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->Shininess;
                    DisplayDouble("            Shininess: ", lKFbxDouble1.Get());
    
                    // Display the Reflectivity
                    lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->ReflectionFactor;
                    DisplayDouble("            Reflectivity: ", lKFbxDouble1.Get());
                }
                else if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
                {
                    // We found a Lambert material. Display its properties.
                    // Display the Ambient Color
                    lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Ambient;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Ambient: ", theColor);
    
                    // Display the Diffuse Color
                    lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Diffuse;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Diffuse: ", theColor);
    
                    // Display the Emissive
                    lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Emissive;
                    theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
                    DisplayColor("            Emissive: ", theColor);
    
                    // Display the Opacity
                    lKFbxDouble1 = ((FbxSurfaceLambert *)lMaterial)->TransparencyFactor;
                    DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
                }
                else
                    DisplayString("Unknown type of Material");
    
                FbxPropertyT<FbxString> lString;
                lString = lMaterial->ShadingModel;
                DisplayString("            Shading Model: ", lString.Get().Buffer());
                DisplayString("");
            }
        }
    }
    void FBXLoader::DisplayMetaDataConnections(FbxObject* pObject)
    {
        int nbMetaData = pObject->GetSrcObjectCount<FbxObjectMetaData>();
        if (nbMetaData > 0)
            DisplayString("    MetaData connections ");
    
        for (int i = 0; i < nbMetaData; i++)
        {
            FbxObjectMetaData* metaData = pObject->GetSrcObject<FbxObjectMetaData>(i);
            DisplayString("        Name: ", (char*)metaData->GetName());
        }
    }
    void FBXLoader::DisplayString(const char* pHeader, const char* pValue /* = "" */, const char* pSuffix /* = "" */)
    {
        FbxString lString;
    
        lString = pHeader;
        lString += pValue;
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::DisplayBool(const char* pHeader, bool pValue, const char* pSuffix /* = "" */)
    {
        FbxString lString;
    
        lString = pHeader;
        lString += pValue ? "true" : "false";
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::DisplayInt(const char* pHeader, int pValue, const char* pSuffix /* = "" */)
    {
        FbxString lString;
    
        lString = pHeader;
        lString += pValue;
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::DisplayDouble(const char* pHeader, double pValue, const char* pSuffix /* = "" */)
    {
        FbxString lString;
        FbxString lFloatValue = (float)pValue;
    
        lFloatValue = pValue <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
        lFloatValue = pValue >= HUGE_VAL ? "INFINITY" : lFloatValue.Buffer();
    
        lString = pHeader;
        lString += lFloatValue;
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::Display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix  /* = "" */)
    {
        FbxString lString;
        FbxString lFloatValue1 = (float)pValue[0];
        FbxString lFloatValue2 = (float)pValue[1];
    
        lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
        lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
        lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
        lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
    
        lString = pHeader;
        lString += lFloatValue1;
        lString += ", ";
        lString += lFloatValue2;
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::Display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix /* = "" */)
    {
        FbxString lString;
        FbxString lFloatValue1 = (float)pValue[0];
        FbxString lFloatValue2 = (float)pValue[1];
        FbxString lFloatValue3 = (float)pValue[2];
    
        lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
        lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
        lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
        lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
        lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
        lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();
    
        lString = pHeader;
        lString += lFloatValue1;
        lString += ", ";
        lString += lFloatValue2;
        lString += ", ";
        lString += lFloatValue3;
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::Display4DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix /* = "" */)
    {
        FbxString lString;
        FbxString lFloatValue1 = (float)pValue[0];
        FbxString lFloatValue2 = (float)pValue[1];
        FbxString lFloatValue3 = (float)pValue[2];
        FbxString lFloatValue4 = (float)pValue[3];
    
        lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
        lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
        lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
        lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
        lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
        lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();
        lFloatValue4 = pValue[3] <= -HUGE_VAL ? "-INFINITY" : lFloatValue4.Buffer();
        lFloatValue4 = pValue[3] >= HUGE_VAL ? "INFINITY" : lFloatValue4.Buffer();
    
        lString = pHeader;
        lString += lFloatValue1;
        lString += ", ";
        lString += lFloatValue2;
        lString += ", ";
        lString += lFloatValue3;
        lString += ", ";
        lString += lFloatValue4;
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }
    void FBXLoader::DisplayColor(const char* pHeader, FbxColor pValue, const char* pSuffix /* = "" */)
    {
        FbxString lString;
    
        lString = pHeader;
        lString += (float)pValue.mRed;
    
        lString += " (red), ";
        lString += (float)pValue.mGreen;
    
        lString += " (green), ";
        lString += (float)pValue.mBlue;
    
        lString += " (blue)";
        lString += pSuffix;
        lString += "\n";
        KLOG_INFO("{0}", lString.Buffer());
    }

    void FBXLoader::DisplayTextureInfo(FbxTexture* pTexture, int pBlendMode)
    {
        FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(pTexture);
        FbxProceduralTexture *lProceduralTexture = FbxCast<FbxProceduralTexture>(pTexture);

        DisplayString("            Name: \"", (char *)pTexture->GetName(), "\"");
        if (lFileTexture)
        {
            DisplayString("            File Name: \"", (char *)lFileTexture->GetFileName(), "\"");
            auto lFileName = std::string((char *)lFileTexture->GetFileName());
            if (m_scene->dummy_actor->m_texture_map.find(lFileName) == m_scene->dummy_actor->m_texture_map.end())
            {
                m_scene->dummy_actor->m_texture_names.push_back(lFileName);
            }
            m_scene->dummy_actor->m_texture_map.insert(std::pair<std::string,int32_t>(std::string(lFileTexture->GetFileName()),static_cast<int32_t>(m_scene->dummy_actor->m_meshes.size())));
        }
        else if (lProceduralTexture)
        {
            DisplayString("            Type: Procedural Texture");
        }
        //DisplayDouble("            Scale U: ", pTexture->GetScaleU());
        //DisplayDouble("            Scale V: ", pTexture->GetScaleV());
        //DisplayDouble("            Translation U: ", pTexture->GetTranslationU());
        //DisplayDouble("            Translation V: ", pTexture->GetTranslationV());
        //DisplayBool("            Swap UV: ", pTexture->GetSwapUV());
        //DisplayDouble("            Rotation U: ", pTexture->GetRotationU());
        //DisplayDouble("            Rotation V: ", pTexture->GetRotationV());
        //DisplayDouble("            Rotation W: ", pTexture->GetRotationW());
        //
        //const char* lAlphaSources[] = { "None", "RGB Intensity", "Black" };
        //
        //DisplayString("            Alpha Source: ", lAlphaSources[pTexture->GetAlphaSource()]);
        //DisplayDouble("            Cropping Left: ", pTexture->GetCroppingLeft());
        //DisplayDouble("            Cropping Top: ", pTexture->GetCroppingTop());
        //DisplayDouble("            Cropping Right: ", pTexture->GetCroppingRight());
        //DisplayDouble("            Cropping Bottom: ", pTexture->GetCroppingBottom());
        //
        //const char* lMappingTypes[] = { "Null", "Planar", "Spherical", "Cylindrical",
        //    "Box", "Face", "UV", "Environment" };
        //
        //DisplayString("            Mapping Type: ", lMappingTypes[pTexture->GetMappingType()]);
        //
        //if (pTexture->GetMappingType() == FbxTexture::ePlanar)
        //{
        //    const char* lPlanarMappingNormals[] = { "X", "Y", "Z" };
        //
        //    DisplayString("            Planar Mapping Normal: ", lPlanarMappingNormals[pTexture->GetPlanarMappingNormal()]);
        //}
        //
        //const char* lBlendModes[] = { "Translucent", "Additive", "Modulate", "Modulate2", "Over", "Normal", "Dissolve", "Darken", "ColorBurn", "LinearBurn",
        //    "DarkerColor", "Lighten", "Screen", "ColorDodge", "LinearDodge", "LighterColor", "SoftLight", "HardLight", "VividLight",
        //    "LinearLight", "PinLight", "HardMix", "Difference", "Exclusion", "Substract", "Divide", "Hue", "Saturation", "Color",
        //    "Luminosity", "Overlay" };
        //
        //if (pBlendMode >= 0)
        //    DisplayString("            Blend Mode: ", lBlendModes[pBlendMode]);
        //DisplayDouble("            Alpha: ", pTexture->GetDefaultAlpha());
        //
        //if (lFileTexture)
        //{
        //    const char* lMaterialUses[] = { "Model Material", "Default Material" };
        //    DisplayString("            Material Use: ", lMaterialUses[lFileTexture->GetMaterialUse()]);
        //}
        //
        //const char* pTextureUses[] = { "Standard", "Shadow Map", "Light Map",
        //    "Spherical Reflexion Map", "Sphere Reflexion Map", "Bump Normal Map" };
        //
        //DisplayString("            Texture Use: ", pTextureUses[pTexture->GetTextureUse()]);
        //DisplayString("");

    }

    void FBXLoader::FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex) {

        if (pProperty.IsValid())
        {
            int lTextureCount = pProperty.GetSrcObjectCount<FbxTexture>();

            for (int j = 0; j < lTextureCount; ++j)
            {
                //Here we have to check if it's layeredtextures, or just textures:
                FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
                if (lLayeredTexture)
                {
                    DisplayInt("    Layered Texture: ", j);
                    int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
                    for (int k = 0; k<lNbTextures; ++k)
                    {
                        FbxTexture* lTexture = lLayeredTexture->GetSrcObject<FbxTexture>(k);
                        if (lTexture)
                        {

                            if (pDisplayHeader) {
                                DisplayInt("    Textures connected to Material ", pMaterialIndex);
                                pDisplayHeader = false;
                            }

                            //NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
                            //Why is that?  because one texture can be shared on different layered textures and might
                            //have different blend modes.

                            FbxLayeredTexture::EBlendMode lBlendMode;
                            lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
                            DisplayString("    Textures for ", pProperty.GetName());
                            DisplayInt("        Texture ", k);
                            DisplayTextureInfo(lTexture, (int)lBlendMode);
                        }

                    }
                }
                else
                {
                    //no layered texture simply get on the property
                    FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
                    if (lTexture)
                    {
                        //display connected Material header only at the first time
                        if (pDisplayHeader) {
                            DisplayInt("    Textures connected to Material ", pMaterialIndex);
                            pDisplayHeader = false;
                        }
                        //Diffuse Color
                        if (pProperty.GetName() == FbxString("DiffuseColor"))
                        {
                            DisplayString("    Textures for ", pProperty.GetName());
                            DisplayInt("        Texture ", j);
                            DisplayTextureInfo(lTexture, -1);
                        }
                    }
                }
            }
        }//end if pProperty

    }


    void FBXLoader::DisplayTexture(FbxGeometry* pGeometry)
    {
        int lMaterialIndex;
        FbxProperty lProperty;
        if (pGeometry->GetNode() == NULL)
            return;
        int lNbMat = pGeometry->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();
        for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++) {
            FbxSurfaceMaterial *lMaterial = pGeometry->GetNode()->GetSrcObject<FbxSurfaceMaterial>(lMaterialIndex);
            bool lDisplayHeader = true;

            //go through all the possible textures
            if (lMaterial) {

                int lTextureIndex;
                FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
                {
                    lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
                    FindAndDisplayTextureInfoByProperty(lProperty, lDisplayHeader, lMaterialIndex);
                }

            }//end if(lMaterial)

        }// end for lMaterialIndex     
    }


}