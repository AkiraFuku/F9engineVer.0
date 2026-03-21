#pragma once
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "Vector4.h"
#include "Vector3.h"
#include <vector>
// 定数：扱える最大ライト数
const int kNumDirectionalLights = 3;
const int kNumPointLights = 3;
const int kNumSpotLights = 3;

class LightManager
{
public: // メンバ関数
    // シングルトンインスタンスの取得
    static LightManager* GetInstance();

    // 初期化
    void Initialize();
    // 更新
    void Update();
    // 描画設定（コマンドリストへのバインド）
    void Draw(UINT rootParameterIndex);

    // 終了処理（明示的に解放したい場合）
    void Finalize();
    // 定数バッファ用構造体 (HLSL側と合わせる)
    struct DirectionalLightData {
        Vector4 color;      // color
        Vector3 direction;  // direction
        float intensity;    // intensity
    };
    struct PointLightData {
        Vector4 color;//ライトの色
        Vector3 position;//ライトの向き
        float intensity;// 明るさ
        float radius;
        float decay;
        float padding[2];
    };
    struct SpotLightData {
        Vector4 color;//ライトの色
        Vector3 position;//ライトの向き
        float intensity;// 明るさ
        Vector3 direction;
        float distance;
        float decay;
        float cosAngle;
        float cosFalloffStart;
        float padding;
    };

    // ライト追加関数
    void AddDirectionalLight(const Vector4& color, const Vector3& direction, float intensity);
    void AddPointLight(const Vector4& color, const Vector3& position, float intensity, float radius, float decay);
    void AddSpotLight(const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart);

    void ClearLights(); // 毎フレームリセット用
    DirectionalLightData& GetDirectionalLight(size_t index);
    PointLightData& GetPointLight(size_t index);
    SpotLightData& GetSpotLight(size_t index);

    // 平行光源 (Directional Light) の設定
    void SetDirectionalLight(size_t index, const Vector4& color, const Vector3& direction, float intensity);

    // 点光源 (Point Light) の設定
    void SetPointLight(size_t index, const Vector4& color, const Vector3& position, float intensity, float radius, float decay);
    void SetPointLightPos(size_t index, const Vector3& position ){pointLights_[index].position=position;};

    // スポットライト (Spot Light) の設定
    void SetSpotLight(size_t index, const Vector4& color, const Vector3& position, float intensity, const Vector3& direction, float distance, float decay, float cosAngle, float cosFalloffStart);
    void SetSpotLightDirection(size_t index,const Vector3& direction){
    spotLights_[index].direction=direction;
    };
    // 有効なライトの数を取得する関数もあると便利です
    size_t GetDirectionalLightCount() const {
        return directionalLights_.size();
    }
    size_t GetPointLightCount() const {
        return pointLights_.size();
    }
    size_t GetSpotLightCount() const {
        return spotLights_.size();
    }

private: // メンバ変数・内部定義
    // シングルトンパターンのためコンストラクタを隠蔽
    LightManager() = default;
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    //// 定数バッファ作成関数
    //void CreateConstBufferResource();



    struct LightCounts {
        int numDirectional;
        int numPoint;
        int numSpot;
        int padding;
    };

    // リソース作成ヘルパー
    void CreateStructuredBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
    void CreateConstBuffer(size_t sizeInBytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource);
    // データ保持用
    std::vector<DirectionalLightData> directionalLights_;
    std::vector<PointLightData> pointLights_;
    std::vector<SpotLightData> spotLights_;

    // GPUリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> dirLightBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> pointLightBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> spotLightBuff_;
    Microsoft::WRL::ComPtr<ID3D12Resource> countBuff_;

    // カウントバッファマップ用
    LightCounts* countData_ = nullptr;
    // シングルトンインスタンス
    static std::unique_ptr<LightManager> instance;
    friend struct std::default_delete<LightManager>;
};
