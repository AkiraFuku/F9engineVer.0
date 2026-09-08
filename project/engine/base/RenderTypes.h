#pragma once

#include "Vector4.h"
#include "Vector3.h"

// 共通レンダリング用定数バッファ構造体
struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Matrix4x4 WorldInverseTranspose;
};

struct CameraForGPU {
    Vector3 worldPosition;
    float farClip;
    Vector3 cameraForward;
    float padding; // 16バイト整列
};

struct DirectionalLight {
    Vector4 color;    // ライトの色
    Vector3 direction; // ライトの向き
    float intensity;   // 明るさ
};

// 汎用マテリアル定数バッファ（Sprite 等で使われているレイアウトに合わせる）
struct MaterialCB {
    Vector4 color;          // RGBA
    int32_t enableLighting; // 0 or 1
    float padding[3];       // 16バイト境界を保つためのパディング
    Matrix4x4 uvTransform;  // UV変換行列（Sprite 用）
};
