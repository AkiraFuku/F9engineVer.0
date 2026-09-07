#pragma once
#include "Vector4.h"
#include "WinApp.h"
#include "Transform.h"
class Camera
{
public:

    Camera ();

    void Update();

    void UpdateView();

    void UpdateViewProjection();


    void SetRotate(const Vector3& rotate) {
        worldTransform_.rotate = rotate;
    }
    void SetTranslate(const Vector3& translate) {
        worldTransform_.translate = translate;
    }
    void SetTransform(const EulerTransform& transForm) {
        worldTransform_ = transForm;
    }
    void SetFovY(const float fovY) {
        this->fovY = fovY;
    }
    void SetAspectRatio(const float aspect) {
        this->aspect = aspect;
    }
    void SetNearCrip(const float nearCrip) {
        this->nearCrip = nearCrip;
    }
    void SetFarCrip(const float farCrip) {
        this->farCrip = farCrip;
    }
    void SetViewMatrix(const Matrix4x4& viewMatrix) {
        this->viewMatrix = viewMatrix;
    }
    //描画範囲の設定
    


    const Vector3& GetRotate()const{return worldTransform_.rotate;}
    const Vector3& GetTranslate()const{return worldTransform_.translate;}
    const EulerTransform& GetTransform()const{ return worldTransform_; };

    const Matrix4x4& GetWorldMatrix()const{return worldMatrix;};
    const Matrix4x4& GetViewMatrix()const{return viewMatrix;};
    const Matrix4x4& GetProjectionMatrix()const{return projectionMatrix;};
    const Matrix4x4& GetViewProtectionMatrix()const{return viewProtectionMatrix;};
    float GetFarCrip() const { return farCrip; }
    EulerTransform worldTransform_;

#ifdef USE_IMGUI
    // --- エディタカメラ（SceneCamera）機能 ---
    void SetEditorMode(bool enable) { isEditorMode_ = enable; }
    bool IsEditorMode() const { return isEditorMode_; }
    void SetMoveSpeed(float speed) { moveSpeed_ = speed; }
    float GetMoveSpeed() const { return moveSpeed_; }
    void SetRotateSpeed(float speed) { rotateSpeed_ = speed; }
    float GetRotateSpeed() const { return rotateSpeed_; }
#endif

private:
#ifdef USE_IMGUI
    void UpdateEditorCamera();

    bool isEditorMode_ = false;       // エディタ操作が有効かどうか
    float moveSpeed_ = 0.5f;          // 移動速度
    float rotateSpeed_ = 0.003f;      // マウス回転感度
    bool isRightDragging_ = false;    // 右ボタンドラッグ中か
    bool isMiddleDragging_ = false;   // 中ボタンドラッグ中か
    EulerTransform initialTransform_; // 初期リセット用
#endif
   
    Matrix4x4 worldMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Matrix4x4 viewProtectionMatrix;
    float fovY ;
    float aspect ;
    float nearCrip ;
    float farCrip ;


};

