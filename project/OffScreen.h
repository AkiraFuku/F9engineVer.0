#pragma once
#include <d3d12.h>     // 先にDirectXの定義を入れる
#include <wrl/client.h>
#include <memory>
#include <vector>
#include "Vector4.h"
#include "Vector2.h"
#include "PSOManager.h"


struct BlurParam
{
    Vector2 center;
    int32_t radius;
    float blurWidth;

};

// --- ビットフラグの定義 ---
enum class PostEffectFlag : uint32_t {
    None             = 0,       // 通常表示
    GrayScale        = 1 << 0,  // 0x01: グレースケール
    Random           = 1 << 1,  // 0x02: ランダムノイズ
    Vignette         = 1 << 2,  // 0x04: ヴィネット
    RadialBlur       = 1 << 3,  // 0x08: ラジアルブラー
    DepthOutline     = 1 << 4,  // 0x10: 深度アウトライン
    LuminanceOutline = 1 << 5,  // 0x20: 輝度アウトライン
    Dissolve         = 1 << 6,  // 0x40: ディゾルブ
    BoxFilter        = 1 << 7   // 0x80: 5x5ボックスフィルタ(平滑化) ← 追加
};// ビット演算子を使いやすくするためのオーバーロード定義
inline PostEffectFlag operator|(PostEffectFlag a, PostEffectFlag b) {
    return static_cast<PostEffectFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline PostEffectFlag operator&(PostEffectFlag a, PostEffectFlag b) {
    return static_cast<PostEffectFlag>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
inline PostEffectFlag& operator|=(PostEffectFlag& a, PostEffectFlag b) {
    a = a | b;
    return a;
}
inline PostEffectFlag& operator&=(PostEffectFlag& a, PostEffectFlag b) {
    a = a & b;
    return a;
}
inline PostEffectFlag operator~(PostEffectFlag a) {
    return static_cast<PostEffectFlag>(~static_cast<uint32_t>(a));
}

class Camera;
class OffScreen
{
public:

    struct MaskMaterial
    {
        std::string textureFilePath;
        uint32_t maskTextureSrvIndex;
    };

    static OffScreen* GetInstance();

    OffScreen(const OffScreen&) = delete;
    OffScreen& operator=(const OffScreen&) = delete;
    friend struct std::default_delete<OffScreen>;

    void Initialize();
    void Finalize();
    void Update();
    void Draw();

    void SetCamera(Camera* camera) {
        camera_ = camera;
    }

    void SetMaskMaterial( std::string textureFilePath) ;

    // --- ビットフラグ操作用 API ---
    void SetEffectFlags(PostEffectFlag flags) { activeFlags_ = flags; }
    void EnableEffect(PostEffectFlag flag) { activeFlags_ |= flag; }
    void DisableEffect(PostEffectFlag flag) { activeFlags_ &= ~flag; }
    void ToggleEffect(PostEffectFlag flag) {
        activeFlags_ = static_cast<PostEffectFlag>(static_cast<uint32_t>(activeFlags_) ^ static_cast<uint32_t>(flag));
    }
    bool IsEffectActive(PostEffectFlag flag) const {
        return (static_cast<uint32_t>(activeFlags_) & static_cast<uint32_t>(flag)) != 0;
    }
    void ClearEffects() { activeFlags_ = PostEffectFlag::None; }

private:
    OffScreen() = default;
    ~OffScreen() = default;

    static std::unique_ptr<OffScreen> instance;

    Camera* camera_;

    // 現在有効なポストエフェクトのフラグ (デフォルトは通常表示)
    PostEffectFlag activeFlags_ = PostEffectFlag::None;

    BlurParam* blurParamData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> blurConstantBuffer_;

struct Material
{
    Matrix4x4 projectionInverse;
    float time;
    uint32_t activeFlags; // シェーダーに送るビットフラグ
    float padding[2];     // 16バイトアラインメント調整
};
    Material* materialData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialConstantBuffer_;

    MaskMaterial MaskMaterial_;

    struct DissolveParm
    {
        float threshold;
        float padding[3]; // 16バイトアラインメントのためのパディング
    };
    DissolveParm* dissolveParamData_;
    Microsoft::WRL::ComPtr<ID3D12Resource> dissolveConstantBuffer_;

    // 前回のコードでこれらがエラーになっていた場合、ここに含まれる型が
    // <d3d12.h> を読み込むことで解決されます
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
};