#pragma once
#include <map>
#include <Model.h>
#include <memory>
#include "DXCommon.h"
class ModelManager
{
public:
    void Initialize();
    static ModelManager* GetInstance();
    void Finalize();
    //Modelロード
    void LoadModel(const std::string& directoryPath,const std::string& filePath);
    //Model検索
    std::shared_ptr<Model> findModel(const std::string& filePath);
    static std::unique_ptr<ModelManager> instance;
    friend struct std::default_delete<ModelManager>;
    void CreateSphereModel(const std::string& modelName, uint32_t subdivision = 16);
    /// <summary>
    /// 板ポリゴンをテクスチャから生成する
    /// </summary>
    /// <param name="modelName">モデルの名前</param>
    /// <param name="textureFilePath">テクスチャファイルのパス</param>
    void CreatePlaneFromTex(const std::string& modelName, const std::string& textureFilePath);
    void CreateBoxModel(const std::string& modelName);


private:
  

    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(ModelManager&) = delete;
    ModelManager& operator=(ModelManager&) = delete;

    std::map<std::string, std::shared_ptr<Model>> models;
};

