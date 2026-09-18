#include "EventTrigger.h"
#include "PrimitiveDrawer.h"
#include <cmath>

void EventTrigger::Initialize(const LevelObjectData& data)
{
    name_      = data.name;
    disabled_  = data.disabled;
    eventName_ = data.trigger.eventName;
    params_    = data.trigger.params;

    // 発火モードの変換
    switch (data.trigger.fireMode) {
    case LevelTriggerData::FireMode::Once:
        fireMode_ = FireMode::Once;
        break;
    case LevelTriggerData::FireMode::Continuous:
        fireMode_ = FireMode::Continuous;
        break;
    case LevelTriggerData::FireMode::EnterExit:
        fireMode_ = FireMode::EnterExit;
        break;
    }

    // コライダー形状 → ワールド座標へ変換
    // (トリガーは静的配置なので Transform + Collider.center = ワールド中心)
    worldCenter_ = {
        data.transform.translation.x + data.collider.center.x,
        data.transform.translation.y + data.collider.center.y,
        data.transform.translation.z + data.collider.center.z
    };

    if (data.collider.shape == LevelColliderData::Shape::Sphere) {
        shape_       = LevelColliderData::Shape::Sphere;
        halfExtents_ = data.collider.size; // x = 半径
    } else {
        shape_       = LevelColliderData::Shape::Box;
        // BOXサイズは「全辺長」として保存されているので半分にする
        halfExtents_ = {
            data.collider.size.x * 0.5f,
            data.collider.size.y * 0.5f,
            data.collider.size.z * 0.5f
        };
    }
}

void EventTrigger::Update(const Vector3& queryPos)
{
    if (disabled_) return;
    // Once モードで既に発火済みなら更新しない
    if (fireMode_ == FireMode::Once && hasFired_) return;

    isInsidePrev_ = isInsideNow_;
    isInsideNow_  = ContainsPoint(queryPos);

    if (!callback_) return;

    switch (fireMode_) {
    case FireMode::Once:
        // 侵入した瞬間に1回だけ発火
        if (isInsideNow_ && !isInsidePrev_) {
            callback_(eventName_, params_);
            hasFired_ = true;
        }
        break;

    case FireMode::Continuous:
        // 領域内にいる間、毎フレーム発火
        if (isInsideNow_) {
            callback_(eventName_, params_);
        }
        break;

    case FireMode::EnterExit:
        // 入ったとき
        if (isInsideNow_ && !isInsidePrev_) {
            auto enterParams = params_;
            enterParams["__event__"] = "enter";
            callback_(eventName_, enterParams);
        }
        // 出たとき
        if (!isInsideNow_ && isInsidePrev_) {
            auto exitParams = params_;
            exitParams["__event__"] = "exit";
            callback_(eventName_, exitParams);
        }
        break;
    }
}

bool EventTrigger::ContainsPoint(const Vector3& point) const
{
    if (shape_ == LevelColliderData::Shape::Sphere) {
        float radius = halfExtents_.x;
        float dx = point.x - worldCenter_.x;
        float dy = point.y - worldCenter_.y;
        float dz = point.z - worldCenter_.z;
        return (dx * dx + dy * dy + dz * dz) <= (radius * radius);
    }

    // AABB (Box)
    return (std::abs(point.x - worldCenter_.x) <= halfExtents_.x) &&
           (std::abs(point.y - worldCenter_.y) <= halfExtents_.y) &&
           (std::abs(point.z - worldCenter_.z) <= halfExtents_.z);
}

void EventTrigger::DebugDraw() const
{
    if (disabled_) return;

    // ─── Box ──────────────────────────────────────────────────────────
    if (shape_ == LevelColliderData::Shape::Box) {
        Vector3 min = {
            worldCenter_.x - halfExtents_.x,
            worldCenter_.y - halfExtents_.y,
            worldCenter_.z - halfExtents_.z
        };
        Vector3 max = {
            worldCenter_.x + halfExtents_.x,
            worldCenter_.y + halfExtents_.y,
            worldCenter_.z + halfExtents_.z
        };

        Vector4 color = isInsideNow_ ? Vector4{ 1.0f, 0.0f, 0.0f, 1.0f }
                                     : Vector4{ 0.0f, 1.0f, 0.5f, 1.0f };

        auto* pd = PrimitiveDrawer::GetInstance();
        // 下面
        pd->DrawLine({ min.x, min.y, min.z }, { max.x, min.y, min.z }, color);
        pd->DrawLine({ max.x, min.y, min.z }, { max.x, min.y, max.z }, color);
        pd->DrawLine({ max.x, min.y, max.z }, { min.x, min.y, max.z }, color);
        pd->DrawLine({ min.x, min.y, max.z }, { min.x, min.y, min.z }, color);
        // 上面
        pd->DrawLine({ min.x, max.y, min.z }, { max.x, max.y, min.z }, color);
        pd->DrawLine({ max.x, max.y, min.z }, { max.x, max.y, max.z }, color);
        pd->DrawLine({ max.x, max.y, max.z }, { min.x, max.y, max.z }, color);
        pd->DrawLine({ min.x, max.y, max.z }, { min.x, max.y, min.z }, color);
        // 柱
        pd->DrawLine({ min.x, min.y, min.z }, { min.x, max.y, min.z }, color);
        pd->DrawLine({ max.x, min.y, min.z }, { max.x, max.y, min.z }, color);
        pd->DrawLine({ max.x, min.y, max.z }, { max.x, max.y, max.z }, color);
        pd->DrawLine({ min.x, min.y, max.z }, { min.x, max.y, max.z }, color);
    }
    // ─── Sphere（8軸簡易描画）─────────────────────────────────────
    // 球の簡易描画は省略（必要であれば PrimitiveDrawer::DrawCircle 等を追加）
}
