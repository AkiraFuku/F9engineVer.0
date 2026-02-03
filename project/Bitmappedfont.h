#pragma once
#include"MathFunction.h"
#include "DrawFunction.h"
#include "Sprite.h"
#include "Camera.h"
#include <memory>

class Bitmappedfont
{
private:

    // ワールド変換データ
    Transform transform_;

    // スプライト
    std::vector<std::unique_ptr<Sprite>>* sprite_;
    std::unique_ptr<Sprite> mySprite_;

    // カメラ
    Camera* camera_ = nullptr;

    // 配列番号
    int index_ = 0;

    Vector2 size_ = {256.0f, 256.0f};

public:
    // 初期化
    void Initialize(std::vector<std::unique_ptr<Sprite>>* sprite,Camera* camera);
    // 更新
    void Update();
    // 描画
    void Draw();
    // 文字設定
    void SetNumber(int index);
    void SetSize(const Vector2 &size);
    // 位置を設定
    void SetPosition(const Vector2& position);
    // コンストラクタとデストラクタ
    Bitmappedfont();
    ~Bitmappedfont();

};

