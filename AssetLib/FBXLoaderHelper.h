#pragma once
#include <memory>
#include <GameScene.h>
#define FBXSDK_SHARED
#include <fbxsdk.h>


namespace assetlib
{


    class FBXLoader
    {
    public:
        ~FBXLoader();
        bool LoadFBXToScene(const std::string& p_file_name,gameplay::GamesScene* p_scene);
        static FBXLoader& GetFbxLoader()
        {
            static FBXLoader l_loader;
            return l_loader;
        };
    private:
        gameplay::GamesScene* m_scene = nullptr;
    private:
        FBXLoader();
        void DisplayTextureNames(FbxProperty &pProperty, FbxString& pConnectionString);
        void DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char * header, int pMatId, int l);
        void DisplayMaterialConnections(FbxMesh* pMesh);
        void DisplayMaterialMapping(FbxMesh* pMesh);
        void DisplayPolygons(FbxMesh* pMesh, gameplay::GameMesh* p_game_mesh, FbxAMatrix* p_transform_matrix);
        void DisplayMaterial(FbxGeometry* pGeometry);
        void DisplayMetaDataConnections(FbxObject* pNode);
        void DisplayString(const char* pHeader, const char* pValue = "", const char* pSuffix = "");
        void DisplayBool(const char* pHeader, bool pValue, const char* pSuffix = "");
        void DisplayInt(const char* pHeader, int pValue, const char* pSuffix = "");
        void DisplayDouble(const char* pHeader, double pValue, const char* pSuffix = "");
        void Display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix = "");
        void Display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
        void DisplayColor(const char* pHeader, FbxColor pValue, const char* pSuffix = "");
        void Display4DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
        void DisplayMesh(FbxNode* pNode, gameplay::GamesScene* p_game_scene);
        void DisplayContent(FbxNode* pNode, gameplay::GamesScene* p_game_scene);
        void DisplayContent(FbxScene* pScene, gameplay::GamesScene* p_game_scene);
        void InitFBXSDK();
        void FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex);
        void DisplayTexture(FbxGeometry* pGeometry);
        void DisplayTextureInfo(FbxTexture* pTexture, int pBlendMode);
    };

    
}


