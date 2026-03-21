#pragma once

#include <Vector4.h>

struct PhysicsNode {
  Vector3 currentPos;  // 今どこにいるか
  Vector3 previousPos; // 1フレーム前にどこにいたか
  float mass;          // 重さ（0なら壁に固定された画鋲）
};