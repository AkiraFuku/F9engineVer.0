#pragma once
#include "Vector4.h"
#include "WinApp.h"
class Camera
{
public:

    Camera ();

    void Update();

    void SetRotate(const Vector3& rotate) {
        transform_.rotate = rotate;
    }
    void SetTranslate(const Vector3& translate) {
        transform_.translate = translate;
    }
    void SetTransform(const Transform& transForm) {
        transform_ = transForm;
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


    const Vector3& GetRotate()const{return transform_.rotate;}
    const Vector3& GetTranslate()const{return transform_.translate;}
    const Matrix4x4& GetWorldMatrix()const{return worldMatrix;};
    const Matrix4x4& GetViewMatrix()const{return viewMatrix;};
    const Matrix4x4& GetProjectionMatrix()const{return projectionMatrix;};
    const Matrix4x4& GetViewProtectionMatrix()const{return viewProtectionMatrix;};
    float GetFarCrip() const { return farCrip; }


private:
    Transform transform_;
    Matrix4x4 worldMatrix;
    Matrix4x4 viewMatrix;
    Matrix4x4 projectionMatrix;
    Matrix4x4 viewProtectionMatrix;
    float fovY ;
    float aspect ;
    float nearCrip ;
    float farCrip ;


};

