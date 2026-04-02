#include "LineDrawer.h"
#include "DXCommon.h"
#include "PSOManager.h"
#include "Logger.h"
#include <cassert>

std::unique_ptr<LineDrawer> LineDrawer::instance_ = nullptr;

LineDrawer* LineDrawer::GetInstance() {
    if (instance_ == nullptr) {
        struct Helper : public LineDrawer {
            Helper() : LineDrawer() {
            }
        };
        instance_ = std::make_unique<Helper>();
    }
    return instance_.get();
}

void LineDrawer::Initialize() {
    PrimitiveDrawer::Initialize();
    AddPSO();
    lineQueue_.reserve(MAX_LINES);
}

void LineDrawer::Finalize() {
    lineQueue_.clear();
    PrimitiveDrawer::Finalize();
}

void LineDrawer::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    if (lineQueue_.size() < MAX_LINES) {
        lineQueue_.push_back({ start, end, color });
    }
}

void LineDrawer::Draw() {
    if (lineQueue_.empty()) {
        return;
    }

    std::vector<VertexData> vertices;
    vertices.reserve(lineQueue_.size() * 2);

    for (const auto& line : lineQueue_) {
        vertices.push_back({ Vector4(line.start.x,line.start.y,line.start.z, 1.0f), line.color });
        vertices.push_back({ Vector4(line.end.x, line.end.y, line.end.z, 1.0f), line.color });
    }

    UpdateVertexBuffer(vertices);

    // 描画処理（既存の Draw 実装を使用）
    PrimitiveDrawer::Draw();

    lineQueue_.clear();
}

void LineDrawer::AddPSO() {
    // LineDrawer 用の PSO 設定
    // 既存の PrimitiveDrawer の AddPSO と同様
}