#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include "Camera.h"
#include "CameraManager.h"

class PbdCommon {
public:
    static PbdCommon* GetInstance();
    void Initialize();
    void Finalize();

    void PbdCommonDraw();

    void SetDefaultCamera(Camera* camera) {
        if (camera) {
            CameraManager::GetInstance()->RegisterCamera("Default", camera);
            CameraManager::GetInstance()->SetActiveGameCamera("Default");
        }
    }

    Camera* GetDefaultCamera() const {
        return CameraManager::GetInstance()->GetActiveGameCamera();
    }

    ID3D12RootSignature* GetRootSignature() const {
        return rootSignature_.Get();
    }

    ID3D12PipelineState* GetPipelineState() const {
        return graphicsPipelineState_.Get();
    }

    friend struct std::default_delete<PbdCommon>;

private:
    PbdCommon() = default;
    ~PbdCommon() = default;
    PbdCommon(const PbdCommon&) = delete;
    PbdCommon& operator=(const PbdCommon&) = delete;

    static std::unique_ptr<PbdCommon> instance_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_;
};
