#pragma once
#include "PrimitiveDrawer.h"
#include "Vector3.h"
#include "Vector4.h"

class LineDrawer : public PrimitiveDrawer {
public:
    static LineDrawer* GetInstance();
    friend struct std::default_delete<LineDrawer>;

    void Initialize() override;
    void Finalize() override;

    void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
    void Draw() override;

protected:
    LineDrawer() = default;
    ~LineDrawer() override = default;
    void AddPSO() override;

private:
    static std::unique_ptr<LineDrawer> instance_;
    struct LineData {
        Vector3 start;
        Vector3 end;
        Vector4 color;
    };

    std::vector<LineData> lineQueue_;
    static constexpr size_t MAX_LINES = 1024;
};