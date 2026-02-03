#include "Bitmappedfont.h"
#include "ModelManager.h"
#include <GameEngine.h>
#include <Framework.h>
#include <cassert>

void Bitmappedfont::SetPosition(const Vector2& position)
{
    transform_.translate.x = position.x;
    transform_.translate.y = position.y;
    mySprite_->SetPosition(position);
}

Bitmappedfont::Bitmappedfont() = default;

void Bitmappedfont::Initialize(std::vector<std::unique_ptr<Sprite>>* sprite, Camera* camera)
{
    // nullポインタチェック
    assert(sprite);
    assert(camera);

    // 引数をメンバ変数に記録
    sprite_ = sprite;
    camera_ = camera;


}

void Bitmappedfont::Update()
{

    if (mySprite_) {
        mySprite_->Update();
    }

}

void Bitmappedfont::Draw()  
{  

    if (mySprite_) {
        mySprite_->Draw();
    }
}

void Bitmappedfont::SetNumber(int index)
{
    index_ = index;

    // 【重要】数字が決まった瞬間に、その数字用のスプライトを「自分専用」として新しく作る
    // これにより、他の桁とスプライトを奪い合わなくなります。
    if (!mySprite_) {
        mySprite_ = std::make_unique<Sprite>();
    }

    // 数字に対応した画像をロードし直す（設計的に許容範囲であればこれが一番確実です）
    std::string filePath = "resources/" + std::to_string(index) + ".png";
    mySprite_->Initialize(filePath.c_str());

    // 保持している座標を反映
    mySprite_->SetPosition(Vector2{ transform_.translate.x, transform_.translate.y });

}

Bitmappedfont::~Bitmappedfont()
{
}
