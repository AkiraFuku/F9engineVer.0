#pragma once
#include "d3d12.h"
#include <wrl/client.h>
#include <cstdint>
#include "DXCommon.h"
#include "Camera.h"
#include "SkyBox.h"
class Object3dCommon
{
public:

    // シングルトン化
    static Object3dCommon* GetInstance();
    void Finalize();

    void Initialize();
    
    void Object3dCommonDraw();
    void SetDefaultCamera(Camera* camera) {
        defaultCamera_ = camera;
    }
    void SetDefaultSkyBox(SkyBox* box) {
       defaultBox_=box;
    }
    Camera* GetDefaultCamera()const {
        return defaultCamera_;
    }
    SkyBox* GetDefaultSkyBox()const {
        return defaultBox_;
    }
    static std::unique_ptr<Object3dCommon> instance;
     friend struct std::default_delete<Object3dCommon>;
private:

    // シングルトンパターン
    Object3dCommon() = default;
    ~Object3dCommon() = default;
    Object3dCommon(const Object3dCommon&) = delete;
    Object3dCommon& operator=(const Object3dCommon&) = delete;

    HRESULT hr_;
    SkyBox* defaultBox_=nullptr;
   

    //ルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_;
    void CreateRootSignature();
    //グラフィックパイプラインステート
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
    void CreatePSO();
    Camera* defaultCamera_ = nullptr;
};

