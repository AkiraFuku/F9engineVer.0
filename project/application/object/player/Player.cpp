#include "Player.h"
#include "Input.h"
#include "ModelManager.h"
#include "imgui.h"
#include "CollisionMask.h"

#include"ThreadManager.h"

#include <cmath>
#include <numbers>

#include "PSOManager.h"
#include "Logger.h"

/// <summary>
/// 初期化
/// </summary>
/// <param name="pos">初期位置</param>
/// <param name="threadManager">ThreadManagerのポインタ</param>
void Player::Initialize(const Vector3& pos, ThreadManager* thread) {

    //ThreadManagerを借りる
    thread_ = thread;

    // モデルの初期化
    object_ = std::make_unique<Object3d>();
    object_->Initialize();
    translate_ = pos;
    object_->SetTranslate(translate_);
    ModelManager::GetInstance()->LoadModel("resources", "player/player.obj");
    object_->SetModel("player/player.obj");

    PsoConfig config{};
    config.vsPath = L"resources/shaders/PLayer/Player.vs.hlsl";
    config.psPath = L"resources/shaders/PLayer/PLayer.ps.hlsl";


    config.rootSignatureGenerator = []() {
        std::vector<D3D12_ROOT_PARAMETER> rootParameters;
        std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler = PSOManager::GetInstance()->StaticSamplers();

        staticSamplers.push_back(sampler);
        D3D12_DESCRIPTOR_RANGE descRangeTexture[1]{};
        descRangeTexture[0].BaseShaderRegister = 0; // t0
        descRangeTexture[0].NumDescriptors = 1;
        descRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        descRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;



        rootParameters.resize(8);



        // Enum定義 (可読性のため)
        enum {
            kMaterial, kTransform, kTexture, DirLight, PointLight, SpotLight, Count, kCamera
        };

        // 0. Material (CBV b0, Pixel)
        rootParameters[kMaterial].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[kMaterial].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[kMaterial].Descriptor.ShaderRegister = 0;

        // 1. Transform (CBV b0, Vertex)
        rootParameters[kTransform].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[kTransform].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        rootParameters[kTransform].Descriptor.ShaderRegister = 0;

        // 2. Texture (Table t0, Pixel)
        rootParameters[kTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[kTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[kTexture].DescriptorTable.pDescriptorRanges = descRangeTexture;
        rootParameters[kTexture].DescriptorTable.NumDescriptorRanges = 1;

        // ★変更: 3. DirectionalLight (SRV t1)
        rootParameters[DirLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameters[DirLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[DirLight].Descriptor.ShaderRegister = 1; // t1

        // ★追加: 4. PointLight (SRV t2)
        rootParameters[PointLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameters[PointLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[PointLight].Descriptor.ShaderRegister = 2; // t2

        // ★追加: 5. SpotLight (SRV t3)
        rootParameters[SpotLight].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
        rootParameters[SpotLight].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[SpotLight].Descriptor.ShaderRegister = 3; // t3

        // ★追加: 6. LightCounts (CBV b3)
        rootParameters[Count].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rootParameters[Count].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rootParameters[Count].Descriptor.ShaderRegister = 3; // b3 (b0,b1,b2は使用済みと仮定、あるいは空いている番号)
        //カメラ
        rootParameters[kCamera].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // 定数バッファビュー
        rootParameters[kCamera].Descriptor.ShaderRegister = 2; // レジスタ番号 2 (b2)
        rootParameters[kCamera].Descriptor.RegisterSpace = 0;
        rootParameters[kCamera].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダーのみ見える

        // シリアライズ
        D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
        descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        descriptionRootSignature.pParameters = rootParameters.data();
        descriptionRootSignature.NumParameters = (UINT)rootParameters.size();
        descriptionRootSignature.pStaticSamplers = staticSamplers.data();
        descriptionRootSignature.NumStaticSamplers = (UINT)staticSamplers.size();


        Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
            assert(false);
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
        hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        assert(SUCCEEDED(hr));



        return rootSignature;
        };
    config.inputLayoutGenerator = []() {
        return std::vector<D3D12_INPUT_ELEMENT_DESC>{
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };
        };
    // 深度設定
    config.depthEnable = true;
    config.depthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

    // PSOManagerに名前を付けて登録
    PSOManager::GetInstance()->RegisterPsoGenerator("PLayer", config);
    object_->SetPsoName("PLayer");

    // 待機状態で初期化
    ChangeState(std::make_unique<PlayerStateIdle>());
}
/// <summary>
/// 終了
/// </summary>
void Player::Finalize() {}
/// <summary>
/// 更新
/// </summary>
void Player::Update() {

    moveVel_ = {0.0f, 0.0f, 0.0f};

    if (state_) {
        state_->Update(this);
    }

    // 糸の相互作用
    UpdateThreadInteraction();

#ifdef USE_IMGUI

    ImGui::Begin("Player Window");

    Vector3 scale = object_->GetScale();
    if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
        object_->SetScale(scale);
    }

    Vector3 rotate = object_->GetRotate();
    if (ImGui::DragFloat3("Rotate", &rotate.x, 0.1f, -360.0f, 360.0f)) {
        object_->SetRotate(rotate);
    }

    Vector3 translate = translate_;
    if (ImGui::DragFloat3("Translate", &translate.x, 0.1f, -100.0f, 100.0f)) {
        translate_ = translate;
    }

    ImGui::End();

#endif

    //// 当たり判定
    //IsCollision();

   

    IsCollisionSDF();

    // 移動距離確定処理
    ResultMove();

    // モデルの更新
    object_->Update();
}
/// <summary>
/// 描画
/// </summary>
void Player::Draw() {
    // モデルの描画
    object_->Draw();
}

/// <summary>
/// 移動処理
/// </summary>
void Player::UpdateMove(Vector3& moveDirection) {
    // 移動方向を蓄積する変数
    moveDirection = {0.0f, 0.0f, 0.0f};

    // キー入力に応じて方向ベクトルを決定
    if (Input::GetInstance()->PushedKeyDown(DIK_D)) {
        moveDirection.x += 1.0f;
    }
    else if (Input::GetInstance()->PushedKeyDown(DIK_A)) {
        moveDirection.x -= 1.0f;
    }
    else
    {
        moveDirection.x = 0.0f;
    }

    if (Input::GetInstance()->PushedKeyDown(DIK_W)) {
        moveDirection.z += 1.0f;
    }
    else if (Input::GetInstance()->PushedKeyDown(DIK_S)) {
        moveDirection.z -= 1.0f;
    }
    else
    {
        moveDirection.z = 0.0f;
    }

    if (moveDirection.x != 0.0f || moveDirection.z != 0.0f) {
        // ベクトルの長さを計算
        float length = std::sqrtf(moveDirection.x * moveDirection.x +
            moveDirection.z * moveDirection.z);
        // 正規化
        if (length != 0.0f) {
            moveDirection.x /= length;
            moveDirection.z /= length;
        }

        // 入力方向から角度を計算
        float targetAngleY = std::atan2(moveDirection.x, moveDirection.z);

        // 目標角度と現在の角度の差分
        float diffrence = targetAngleY - rotationY_;

        // 差分を-π ~ πの範囲に収める(最短経路で旋回)
        while (diffrence > std::numbers::pi_v<float>) {
            diffrence -= 2.0f * std::numbers::pi_v<float>;
        }
        while (diffrence < -std::numbers::pi_v<float>) {
            diffrence += 2.0f * std::numbers::pi_v<float>;
        }

        // 現在の角度に差分の一部を加算
        rotationY_ += diffrence * kTurnSpeed;

        // モデルに適用
        rotate_ = {0.0f, rotationY_, 0.0f};
        object_->SetRotate(rotate_);

        // 正規化した方向に指定速度を掛けて加算
        moveVel_.x += moveDirection.x * velocity_.x;
        moveVel_.z += moveDirection.z * velocity_.z;
    }


}

void Player::IsCollision()
{

    float nextX = translate_.x + moveVel_.x;
    float nextZ = translate_.z + moveVel_.z;

    //// 移動先が壁かどうかチェック
    //if (CollisionMask::GetInstance()->IsCollisionWall(nextX, translate_.z, kWidth)) 
    //{
    //    // 壁なら移動をキャンセル（または押し戻し）
    //    moveVel_.x = 0.0f;
    //}

    //if (CollisionMask::GetInstance()->IsCollisionWall(translate_.x, nextZ, kWidth))
    //{
    //    // 壁なら移動をキャンセル（または押し戻し）
    //    moveVel_.z = 0.0f;
    //}

    /*float nextX = translate_.x + moveVel_.x;
    if (CollisionMask::GetInstance()->IsWall(nextX, translate_.z))
    {
        moveVel_.x = 0.0f;
    }

    float nextZ = translate_.z + moveVel_.z;
    if (CollisionMask::GetInstance()->IsWall(translate_.x, nextZ))
    {
        moveVel_.z = 0.0f;
    }*/

    float threadWalkRadius_ = 0.5f;

    // --- X軸方向の判定 ---
    // 移動先が壁かどうかチェック
    if (CollisionMask::GetInstance()->IsCollisionWall(nextX, translate_.z, kWidth))
    {
        // 壁にぶつかった！でも糸の上ならセーフにする
        Vector3 nextPos = {nextX, translate_.y, translate_.z};
        if (!thread_->IsOnThread(nextPos, threadWalkRadius_)) {
            // 糸の上でもないので移動をキャンセル
            moveVel_.x = 0.0f;
        }
    }

    // --- Z軸方向の判定 ---
    if (CollisionMask::GetInstance()->IsCollisionWall(translate_.x, nextZ, kWidth))
    {
        // 壁にぶつかった！でも糸の上ならセーフにする
        Vector3 nextPos = {translate_.x, translate_.y, nextZ};
        if (!thread_->IsOnThread(nextPos, threadWalkRadius_)) {
            // 糸の上でもないので移動をキャンセル
            moveVel_.z = 0.0f;
        }
    }
}

void Player::ResultMove()
{
    translate_ += moveVel_;
    object_->SetTranslate(translate_);
}

void Player::IsCollisionSDF()
{
    Vector3 nextPos = {};
    nextPos.x = translate_.x + moveVel_.x;
    nextPos.z = translate_.z + moveVel_.z;

    // 2. 矩形の四隅のオフセット（中心からの距離）
    float hW = kWidth * 0.5f;
    float hH = kHeight * 0.5f;

    for (int i = 0; i < 3; i++)
    {
        // 四隅の座標リスト
        Vector2 corners[4] = {
            { nextPos.x - hW, nextPos.z - hH }, // 左下
            { nextPos.x + hW, nextPos.z - hH }, // 右下
            { nextPos.x - hW, nextPos.z + hH }, // 左上
            { nextPos.x + hW, nextPos.z + hH }  // 右上
        };

        // 3. 四隅の中で「最も壁に近い点」を探す
        float minDist = 10000.0f;
        Vector2 targetCorner = { nextPos.x, nextPos.z };

        for (const auto& corner : corners) {
            float d = CollisionMask::GetInstance()->GetSDFValue(corner.x, corner.y);
            if (d < minDist) {
                minDist = d;
                targetCorner = corner;
            }
        }

        // 4. 衝突判定（最も近い点が「中」に入っていたら）
        // 矩形判定の場合、理想的な距離（閾値）は 0 です。
        if (minDist < 0.1f) {
            // 最もめり込んでいる点の法線を取得
            Vector2 normal = CollisionMask::GetInstance()->GetSDFNormal(targetCorner.x, targetCorner.y);

            if (std::abs(normal.x) > 0.0001f || std::abs(normal.y) > 0.0001f)
            {
                // 押し戻し量
                float pushBack = 0.1f - minDist;

                // 座標を補正
                moveVel_.x += normal.x * pushBack;
                moveVel_.z += normal.y * pushBack;

                // 速度の射影（滑り処理）
                float dot = moveVel_.x * normal.x + moveVel_.z * normal.y;
                if (dot < 0) {
                    moveVel_.x -= dot * normal.x;
                    moveVel_.z -= dot * normal.y;
                }
            }
        }
    }

    

//    // 1. 移動後の座標でのSDF値（距離）を取得
//    float dist = CollisionMask::GetInstance()->GetSDFValue(nextPos.x, nextPos.z);
//
//    // 2. もし半径（プレイヤーの大きさ）より距離が小さければ衝突
//    float playerRadius = kWidth * 0.5f;
//    if (dist < playerRadius) {
//        // 3. 「法線（勾配）」を求める
//        // 周囲のSDF値の差から、どっちに動けば距離が増えるか（壁から離れるか）がわかる
//        Vector2 normal = CollisionMask::GetInstance()->GetSDFNormal(nextPos.x, nextPos.z);
//
//        // 4. 距離が足りない分だけ、法線方向に一気に押し戻す！
//        float pushBack = playerRadius - dist;
//
//        // 座標を直接補正する
//        nextPos.x += normal.x * pushBack;
//        nextPos.z += normal.y * pushBack;
//
//        // 【重要】速度ベクトルを壁に沿わせる（法線方向の速度を消す）
//        // これをしないと、壁に向かって進み続けようとしてガタつきます
//        float dot = moveVel_.x * normal.x + moveVel_.z * normal.y;
//        if (dot < 0) { // 壁に向かう成分がある場合のみ
//            moveVel_.x -= dot * normal.x;
//            moveVel_.z -= dot * normal.y;
//        }
//    }
//
//#ifdef USE_IMGUI
//    ImGui::Begin("Collision Debug");
//    float worldX = translate_.x;
//    float worldZ = translate_.z;
//
//    auto mask = CollisionMask::GetInstance()->GetMaskData(static_cast<int>(CollisionMask::GetInstance()->GetCurrentMaskMap()));
//    float u = (worldX - mask->min_.x) / (mask->max_.x - mask->min_.x);
//    float v = (worldZ - mask->min_.y) / (mask->max_.y - mask->min_.y);
//
//    ImGui::Text("World Pos: %.2f, %.2f", worldX, worldZ);
//    ImGui::Text("UV Pos: %.3f, %.3f", u, v); // これが 0.0~1.0 になっているか
//    ImGui::Text("Pixel: %d, %d", (int)(u * mask->textureData.widthX), (int)(v * mask->textureData.widthZ));
//    ImGui::Text("SDF Dist: %.2f", CollisionMask::GetInstance()->GetSDFValue(worldX, worldZ));
//    ImGui::End();
//#endif
}

/// <summary>
/// 状態遷移
/// </summary>
/// <param name="newState">次の状態</param>
void Player::ChangeState(std::unique_ptr<IPlayerState> newState)
{
    state_ = std::move(newState);
    state_->Initialize(this);
}

/// <summary>
/// 糸を発射する処理
/// </summary>
void Player::FireThread() {
    if (!thread_) {
        return;
    }

    // 向いている方向
    Vector3 forward = GetForward();
    // プレイヤーの位置
    Vector3 playerPos = GetPosition();

    // 始点（プレイヤーの中心より少し上から出すなど、調整可能）
    Vector3 startPos = {playerPos.x, playerPos.y, playerPos.z};

    // 終点（向いている方向に距離を掛けた位置）
    float threadLength = 10.0f; // 糸を飛ばす距離を指定
    Vector3 endPos = {
        startPos.x + forward.x * threadLength,
        startPos.y + forward.y * threadLength,
        startPos.z + forward.z * threadLength
    };

    // ThreadManagerに糸の生成を依頼
    thread_->AddThread(startPos, endPos);
}

void Player::SetPosition(const Vector3& pos)
{
    translate_ = pos;
    object_->SetTranslate(translate_);
}

// 向いている方向
Vector3 Player::GetForward() const
{
    return {std::sin(rotationY_), 0.0f, std::cos(rotationY_)};
}

AABB Player::GetAABB() const
{
    Vector3 worldPos = GetPosition();
    AABB aabb;

    aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
    aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

    return aabb;
}

/// <summary>
/// 糸の相互作用
/// </summary>
void Player::UpdateThreadInteraction()
{
    if (thread_) {
        
        float influenceRadius = 1.5f; // どれくらい広い範囲の糸を巻き込んでたわませるか
        float playerWeight = 0.05f;   // どれくらい下に沈ませるか

        // 毎フレーム、現在位置の周りの糸を下へ押し下げる
        thread_->ApplyPlayerWeight(translate_, influenceRadius, playerWeight);

        float threadY = 0.0f;
        float walkRadius = 1.0f; // 糸の上に乗れる半径（IsOnThreadで使っている値と同じくらい）

        onThread_ = thread_->GetThreadHeight(translate_, walkRadius, threadY);

        // 糸の上にいるなら、Y座標を上書きする
        if (thread_->GetThreadHeight(translate_, walkRadius, threadY)) {

            // プレイヤーの原点が中心にあるか足元にあるかで調整が必要
            float playerOffsetY = 0.0f; // もし体が糸に半分めり込むなら 1.0f など足して調整

            translate_.y = threadY + playerOffsetY;
        }
    }
}

Matrix4x4 Player::GetWorldMatrix() const
{
    Matrix4x4 worldMatrix = MakeAfineMatrix(scale_, rotate_, translate_);
    return worldMatrix;
}