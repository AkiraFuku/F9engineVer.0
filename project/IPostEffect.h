#pragma once
#include <d3d12.h>
#include <memory>
#include "PSOManager.h" // RootSignatureBuilderが含まれるヘッダー

class IPostEffect {
public:
    virtual ~IPostEffect() = default;

    // 1. RootSignature のパラメータ定義（参照渡しで受け取ってビルダーに追加する）
    virtual void SetupRootSignature(RootSignatureBuilder& builder) = 0;

    // 2. パラメータの更新（毎フレーム実行）
    virtual void Update() = 0;

    // 3. 描画時のバインド（コマンドリストにCBVやDescriptorTableをセット）
    virtual void BindCommand(ID3D12GraphicsCommandList* commandList) = 0;

    // --- 補助機能（マネージャーから呼び出しやすくするための共通インターフェース）---
    virtual const char* GetName() const = 0;
    virtual void Initialize() = 0; // CBVの生成やマッピングなど
};