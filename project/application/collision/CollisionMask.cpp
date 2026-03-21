#define NOMINMAX
#include "CollisionMask.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "StringUtility.h"
#include "Vector4Function.h"
#include "ImGuiManager.h"

std::unique_ptr<CollisionMask, CollisionMask::Deleter> CollisionMask::instance_ = nullptr;

CollisionMask* CollisionMask::GetInstance() 
{
    if (instance_ == nullptr)
    {
        instance_.reset(new CollisionMask());
    }
    return instance_.get();
};

void CollisionMask::Finalize()
{
    instance_.reset();
}

void CollisionMask::Initialize() 
{
   
    maskDatas_.resize(3);

    for (size_t i = 0; i < 3; i++)
    {

        std::unique_ptr<MaskData> maskData{};

        std::unique_ptr<Object3d>object = std::make_unique<Object3d>();
        object->Initialize();
        object->SetRotate(Vector3{ -90.0f / 180.0f * PI, 0.0f, 0.0f });
        object->SetTranslate(translate_);
        
        maskDatas_[i] = std::make_unique<MaskData>();
        maskDatas_[i]->object = std::move(object);
        

    }

    LoadFromFile("resources/Mask/Mask.png", maskDatas_[0]->textureData);
    maskDatas_[0]->name = "mapMaskData";
    ModelManager::GetInstance()->CreatePlaneFromTex(maskDatas_[0]->name, "resources/Mask/Mask.png");
    LoadFromFile("resources/Mask/Mask(1).png", maskDatas_[1]->textureData);
    maskDatas_[1]->name = "mapMaskData1";
    ModelManager::GetInstance()->CreatePlaneFromTex(maskDatas_[1]->name, "resources/Mask/Mask(1).png");
    LoadFromFile("resources/Mask/Mask(2).png", maskDatas_[2]->textureData);
    maskDatas_[2]->name = "mapMaskData2";
    ModelManager::GetInstance()->CreatePlaneFromTex(maskDatas_[2]->name, "resources/Mask/Mask(2).png");

    for (size_t i = 0; i < 3; i++)
    {
        GenerateSDF(maskDatas_[i].get());

        maskDatas_[i]->object->SetModel(maskDatas_[i]->name);

        auto model = ModelManager::GetInstance()->findModel(maskDatas_[i]->name);

        maskDatas_[i]->max_ = model->GetModelData().vertices[1].position;
        maskDatas_[i]->min_ = model->GetModelData().vertices[2].position;
    }
    
    currentMaskMap_ = CollisionMask::MaskMap::Map2;

    maskMapRequest_ = CollisionMask::MaskMap::Unknown;
}

void CollisionMask::Update() 
{
    if(maskMapRequest_ != MaskMap::Unknown)
    {
        currentMaskMap_ = maskMapRequest_;
        maskMapRequest_ = MaskMap::Unknown;
    }

    ImGui::Begin("MaskMap Setting");

    int maskMapIndex = static_cast<int>(currentMaskMap_);
    const char* items[] = {"Map1", "Map2", "Map3"};
    if (ImGui::Combo("Mask Map", &maskMapIndex, items, IM_ARRAYSIZE(items)))
    {
        SetMaskMapRequest(static_cast<MaskMap>(maskMapIndex));
    }

    ImGui::End();

    maskDatas_[static_cast<int>(currentMaskMap_)]->object->Update();
}

void CollisionMask::Draw() 
{
    maskDatas_[static_cast<int>(currentMaskMap_)]->object->Draw();
}

bool CollisionMask::LoadFromFile(const std::string& filePath, TextureData& textureData)
{
    DirectX::ScratchImage scrachImage{};
    std::wstring filePathW = StringUtility::ConvertString(filePath);

    // 1. ロード
    HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scrachImage);
    if (FAILED(hr)) return false;

    TextureManager::GetInstance()->LoadTexture(filePath);

    // 2. 判定しやすいように強制変換 (RGBA 8bit)
    DirectX::ScratchImage converted{};
    hr = DirectX::Convert(
        *scrachImage.GetImage(0, 0, 0),
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DirectX::TEX_FILTER_DEFAULT, 0,
        converted
    );

    if (FAILED(hr)) return false;

    const DirectX::Image* image = converted.GetImage(0, 0, 0);
    textureData.widthX = static_cast<int>(image->width);
    textureData.widthZ = static_cast<int>(image->height);
    textureData.data.assign(textureData.widthX * textureData.widthZ, 0);

    for (size_t y = 0; y < image->height; ++y) {
        for (size_t x = 0; x < image->width; ++x) {
            // ピクセル位置のポインタを計算 (RGBA8を想定)
            uint8_t* pPixel = image->pixels + (y * image->rowPitch) + (x * 4);

            // R, G, B の平均値などで判定 (あるいはAlpha値)
            uint8_t brightness = (pPixel[0] + pPixel[1] + pPixel[2]) / 3;
            textureData.data[y * textureData.widthX + x] = brightness;
        }
    }

    return false;
}

void CollisionMask::GenerateSDF(MaskData* mask)
{
    int w = mask->textureData.widthX;
    int h = mask->textureData.widthZ;
    mask->sdfData.resize(w * h); // float型の配列を用意

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            // 全ピクセルを走査して最短距離を計算
            // (本来は 8-point SSEDT 等の高速アルゴリズムを使いますが、まずは二重ループで)
            float minDist = FindNearestWallDist(x, y, mask);
            mask->sdfData[y * w + x] = minDist;
        }
    }
}

float CollisionMask::FindNearestWallDist(int startX, int startZ, MaskData* mask)
{
    int w = mask->textureData.widthX;
    int h = mask->textureData.widthZ;

    // 探索する最大半径（これより遠い壁は無視していいことにする）
    // マップ全体のサイズに合わせて調整。例：32ピクセル分など
    const int maxSearchRange = 64;
    float minSqDist = static_cast<float>(maxSearchRange * maxSearchRange);
    bool found = false;

    // 現在のピクセルがすでに壁（黒）なら、距離は 0 (または負の深さ)
    if (mask->textureData.data[startZ * w + startX] < 128) {
        return 0.0f; // 簡易版なので壁の中は0
    }

    // 周囲を矩形状に探索
    for (int dz = -maxSearchRange; dz <= maxSearchRange; ++dz) {
        for (int dx = -maxSearchRange; dx <= maxSearchRange; ++dx) {
            int targetX = startX + dx;
            int targetZ = startZ + dz;

            // 範囲外チェック
            if (targetX < 0 || targetX >= w || targetZ < 0 || targetZ >= h) continue;

            // そのピクセルが壁（黒）かどうか
            if (mask->textureData.data[targetZ * w + targetX] < 128) {
                float sqDist = static_cast<float>(dx * dx + dz * dz);
                if (sqDist < minSqDist) {
                    minSqDist = sqDist;
                    found = true;
                }
            }
        }
    }

    return found ? std::sqrt(minSqDist) : static_cast<float>(maxSearchRange);
}

float CollisionMask::GetSDFValue(float worldX, float worldZ)
{
    auto& maskData = maskDatas_[static_cast<int>(currentMaskMap_)];

    // 1. ワールド座標を画像上の浮動小数点の座標 (u, v) に変換
    float u = (worldX - maskData->min_.x) / (maskData->max_.x - maskData->min_.x) 
        * (maskData->textureData.widthX - 1);
    float v = (worldZ - maskData->min_.y) / (maskData->max_.y - maskData->min_.y) 
        * (maskData->textureData.widthZ - 1); // 反転が必要なら 1.0f-

    // 2. 周辺4ピクセルの座標を特定
    int x0 = static_cast<int>(u);
    int y0 = static_cast<int>(v);
    int x1 = std::min(x0 + 1, maskData->textureData.widthX - 1);
    int y1 = std::min(y0 + 1, maskData->textureData.widthZ - 1);

    // 3. ピクセル内の余り（0.0〜1.0）
    float fx = u - x0;
    float fy = v - y0;

    // 4. 4点のSDF値を線形補間（これによってギザギザが消える）
    float d00 = maskData->sdfData[y0 * maskData->textureData.widthX + x0];
    float d10 = maskData->sdfData[y0 * maskData->textureData.widthX + x1];
    float d01 = maskData->sdfData[y1 * maskData->textureData.widthX + x0];
    float d11 = maskData->sdfData[y1 * maskData->textureData.widthX + x1];

    float d0 = d00 * (1 - fx) + d10 * fx;
    float d1 = d01 * (1 - fx) + d11 * fx;

    return d0 * (1 - fy) + d1 * fy;
}

Vector2 CollisionMask::GetSDFNormal(float worldX, float worldZ)
{
    // 微小な差分を使って「傾き（勾配）」を求める
    const float delta = 0.1f;

    // X方向とZ方向でそれぞれ「ちょっと動いた時の距離の変化」を見る
    float dx = GetSDFValue(worldX + delta, worldZ) - GetSDFValue(worldX - delta, worldZ);
    float dz = GetSDFValue(worldX, worldZ + delta) - GetSDFValue(worldX, worldZ - delta);

    float length = std::sqrtf(dx * dx + dz * dz);
    if(length < 1e-5f) {
        // 勾配がほとんどない（距離場が平坦）場合は、適当な法線を返す
        return Vector2(0.0f, 0.0f);
    }

    return Vector2{ dx / length, dz / length };
}



bool CollisionMask::IsWall(float x, float z) const
{


    Vector4 min_ = maskDatas_[static_cast<int>(currentMaskMap_)]->min_;
    Vector4 max_ = maskDatas_[static_cast<int>(currentMaskMap_)]->max_;
    int widthX = maskDatas_[static_cast<int>(currentMaskMap_)]->textureData.widthX;
    int widthZ = maskDatas_[static_cast<int>(currentMaskMap_)]->textureData.widthZ;

    float u = ((x - min_.x) / (max_.x - min_.x)) * widthX;
    float v = ((z - min_.y) / (max_.y - min_.y)) * widthZ;

    int ix = static_cast<int>(u);
    int iz = static_cast<int>(v);

    // 画面外は壁として扱う
    if (ix < 0 || ix >= widthX || iz < 0 || iz >= widthZ) return true;

    // インデックス計算（1ピクセル1バイトの場合）
    return maskDatas_[static_cast<int>(currentMaskMap_)]->textureData.data[iz * widthX + ix] < 128; // 閾値で判定
}

bool CollisionMask::IsCollisionWall(const float& x, const float& z, const float& width)
{
    Vector4 min_ = maskDatas_[static_cast<int>(currentMaskMap_)]->min_;
    Vector4 max_ = maskDatas_[static_cast<int>(currentMaskMap_)]->max_;
    int widthX = maskDatas_[static_cast<int>(currentMaskMap_)]->textureData.widthX;
    int widthZ = maskDatas_[static_cast<int>(currentMaskMap_)]->textureData.widthZ;

    float left  = x - (width / 2.0f);
    float right = x + (width / 2.0f);
    float front = z - (width / 2.0f);
    float back = z + (width / 2.0f);
  
    float v1 = (front - min_.y) / (max_.y - min_.y) * widthZ;
    float v2 = (back - min_.y) / (max_.y - min_.y) * widthZ;
    int minIZ = std::clamp(static_cast<int>(std::min(v1, v2)), 0, widthZ - 1);
    int maxIZ = std::clamp(static_cast<int>(std::max(v1, v2)), 0, widthZ - 1);

    float u1 = (left - min_.x) / (max_.x - min_.x) * widthX;
    float u2 = (right - min_.x) / (max_.x - min_.x) * widthX;
    int minIX = std::clamp(static_cast<int>(std::min(u1, u2)), 0, widthX - 1);
    int maxIX = std::clamp(static_cast<int>(std::max(u1, u2)), 0, widthX - 1);

    // 範囲内のピクセルを走査
    for (int iz = minIZ; iz <= maxIZ; ++iz) {
        for (int ix = minIX; ix <= maxIX; ++ix) 
        {
            // 黒ピクセルがあれば衝突
            if (maskDatas_[static_cast<int>(currentMaskMap_)]->textureData.data[iz * widthX + ix] < 128)
            {
                ImGui::Begin("Debug");
                ImGui::Text("Hit at Pixel: X=%d, Z=%d", ix, iz);
                ImGui::End();

                return true;
            }
        }
    }

    return false;
}

