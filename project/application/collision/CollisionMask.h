#pragma once
#include "Camera.h"
#include "Model.h"
#include "Object3D.h"
#include "DrawFunction.h"

class CollisionMask
{
public:

    struct TextureData
    {
        std::vector<uint8_t> data;
        int widthX, widthZ;
    };

    struct MaskData
    {
        // 名前
        std::string name;
        std::unique_ptr<Object3d> object;
        TextureData textureData;
        Vector4 min_, max_;
        std::vector<float> sdfData;
    };

    enum class MaskMap
    {
        Map1,
        Map2,
        Map3,

        Unknown,
    };

public: 

    // シングルトンインスタンスの取得
    static CollisionMask* GetInstance();
    /// <summary>
    /// 終了
    /// </summary>
    void Finalize();

public:

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();
    
   

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

public:

    // マスクデータを読み込む
    bool LoadFromFile(const std::string& filePath, TextureData& textureData);

    // マスクデータから簡易的な距離場を生成
    void GenerateSDF(MaskData* mask);

    float FindNearestWallDist(int startX, int startZ, MaskData* mask);

    MaskData* GetMaskData(int num){ return maskDatas_[num].get(); }

    float GetSDFValue(float worldX, float worldZ);

    Vector2 GetSDFNormal(float worldX, float worldZ);

    // 特定座標が壁かどうかを判定
    bool IsWall(float x, float z) const;

    bool IsCollisionWall(const float& x, const float& z, const float& width);

    int GetCurrentMaskMap() const { return static_cast<int>(currentMaskMap_); }


public:

    void SetMaskMapRequest(const MaskMap& maskMapRequest){ maskMapRequest_ = maskMapRequest; }

private: // シングルトンインスタンス

    // unique_ptr が delete するために使用する構造体
    struct Deleter
    {
        void operator()(CollisionMask* p) const
        {
            // クラス内部のスコープなので private なデストラクタを呼べる
            delete p;
        }
    };

    // インスタンス
    static std::unique_ptr<CollisionMask, Deleter> instance_;

    CollisionMask() = default;
    ~CollisionMask() = default;
    CollisionMask(const CollisionMask&) = delete;
    CollisionMask& operator=(const CollisionMask&) = delete;

private: 

    std::vector<std::unique_ptr<MaskData>> maskDatas_;

    Vector3 translate_ = { 0.0f, 0.0f, 0.0f };

    MaskMap currentMaskMap_ = MaskMap::Map1;

    MaskMap maskMapRequest_ = MaskMap::Unknown;

};

