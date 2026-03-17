#pragma once
#include <map>
#include <Model.h>
#include <memory>
#include "DXCommon.h"
class ModelCommon;
class ModelManager
{
public:
    void Initialize();
    static ModelManager* GetInstance();
    void Finalize();
    //Modelロード
    void LoadModel(const std::string& filePath);
    //Model検索
    std::unique_ptr<Model> findModel(const std::string& filePath);
    static std::unique_ptr<ModelManager> instance_;
    friend struct std::default_delete<ModelManager>;
    void CreateSphereModel(const std::string& modelName, uint32_t subdivision = 16);
private:
    std::unique_ptr<ModelCommon> modelCommon_;

    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(ModelManager&) = delete;
    ModelManager& operator=(ModelManager&) = delete;

    std::map<std::string, std::unique_ptr<Model>> models;
};

