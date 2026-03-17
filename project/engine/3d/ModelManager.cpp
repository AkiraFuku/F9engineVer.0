#include "ModelManager.h"
#include "ModelCommon.h"
std::unique_ptr<ModelManager> ModelManager::instance_ = nullptr;
void ModelManager::Initialize() {
 

}
ModelManager* ModelManager::GetInstance() {
  /*  if (instance == nullptr) {
        instance = std::make_unique<ModelManager>();
    }*/
    if (instance_ == nullptr) {
        // privateコンストラクタを呼び出せるヘルパー構造体
        struct Helper : public ModelManager {
            Helper() : ModelManager() {}
        };
        instance_ = std::make_unique<Helper>();
    }
    return instance_.get();
}
void ModelManager::Finalize() {

    models.clear(); 
   // instance.reset();
}

void ModelManager::LoadModel(const std::string& filePath)
{
    // 読み込み済か確認
    if (models.contains(filePath)) return;
    // 読み込み・初期化
    std::unique_ptr<Model> model = std::make_unique<Model>();
    model->Initialize("resources", filePath);
    // 格納
    models.insert(std::make_pair(filePath, std::move(model)));
}

std::shared_ptr<Model> ModelManager::findModel(const std::string& filePath)
{
    if (models.contains(filePath)) {
        return models.at(filePath);

        // 既存のモデルを新しいunique_ptrで返すためにコピーコンストラクタを使用
        //return std::make_unique<Model>(*models.at(filePath));

    }


    LoadModel(filePath);
    if (models.contains(filePath)) {
      return models.at(filePath);
    }
    return nullptr;
}

void ModelManager::CreateSphereModel(const std::string& modelName, uint32_t subdivision)
{
    // 既に同じ名前で登録されていたら何もしない（あるいは上書き）
    if (models.contains(modelName)) {
        return;
    }

    // 1. Modelクラスの便利関数を使って球体を生成
    // ※Model::CreateSphereの実装が必要です（前回の回答参照）
    // Model::CreateSphereが rawポインタ(Model*)を返す前提で shared_ptr で受け取ります
    std::shared_ptr<Model> model(Model::CreateSphere(subdivision));

    // 2. マップに登録
    // これで "Sphere" などの名前で検索できるようになります
 models.insert(std::make_pair(modelName, std::move(model)));
}
