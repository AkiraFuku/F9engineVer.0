#pragma once
#include "ParticleEmitter.h"
#include "ParticleManager.h"
#include <functional>



 // 変形演出用の関数ポインタ（ラムダ式）を外部公開
// 1. 円柱（Cylinder）用の初期化と更新

//平たい状態から伸びてまた縮む
//上面と下面の円形の面が伸びてまた縮むような演出

extern ParticleManager::ParticleEmitterFunc CylinderInitFunc;
extern ParticleManager::ParticleUpdateFunc  CylinderUpdateFunc;

// 2. リング（Ring）用の初期化と更新

//拡大しながら薄くなって消えるような演出

extern ParticleManager::ParticleEmitterFunc RingInitFunc;
extern ParticleManager::ParticleUpdateFunc  RingUpdateFunc;

// 3. 平面（Plane）用の初期化と更新
extern ParticleManager::ParticleEmitterFunc PlaneInitFunc;
extern ParticleManager::ParticleUpdateFunc  PlaneUpdateFunc;
