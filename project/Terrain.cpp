#include "Terrain.h"
#include "DrawFunction.h"
void Terrain::Initialize(const std::string& modelPath)
{
    object_ = std::make_unique<Object3d>();
    object_->Initialize();
    object_->SetModel(modelPath);
}

void Terrain::Update()
{
    object_->Update();
}

void Terrain::Draw()
{
    object_->Draw();
}

void Terrain::OnCollision(ICollider* other)
{
    // 地形は基本的に動かないので、衝突時の特別な処理は不要ですが、必要に応じてここにロジックを追加できます。
    other; // 相手にも衝突を通知

}

void Terrain::UpdateBoundingBox(Vector3 max, Vector3 min)
{
    // モデルのメッシュ情報を取得
    Model* model = object_->GetModel();
    // モデルが存在しない場合はデフォルトのAABBを設定
    if (!model) {
        boundingBox_.max = { 0.5f, 0.5f, 0.5f }; // デフォルトの最大点
        boundingBox_.min = { -0.5f, -0.5f, -0.5f }; // デフォルトの最小点
        return;
    }

    // モデルのメッシュ情報からAABBを更新
    boundingBox_.max = max;
    boundingBox_.min = min;
}

bool Terrain::TestCollision(const ICollider* other) const
{
    return false;
}

bool Terrain::TestRay(const Ray& ray, RayCastHit* outHit)
{
    // モデルのメッシュ情報を取得
    Model* model = object_->GetModel();
    if (!model) return false;

    const auto& vertices = model->GetVertices();
    const auto& indices = model->GetIndices();
    const Matrix4x4& worldMat = object_->GetWorldMatrix();

    float closestDistance = FLT_MAX;
    bool isHit = false;

    // 全てのポリゴン（三角形）に対して判定
    for (size_t i = 0; i < indices.size(); i += 3) {
        // ワールド座標に変換

        Triangle triangle;
        triangle.vertices[0] = Transform(vertices[indices[i]].position.ToVector3(), worldMat);
        triangle.vertices[1] = Transform(vertices[indices[i + 1]].position.ToVector3(), worldMat);
        triangle.vertices[2] = Transform(vertices[indices[i + 2]].position.ToVector3(), worldMat);

        float distance;
        // レイと三角形の交差判定 (MathFunction等に実装が必要)
        if (CheckRayTriangle(ray, triangle, &distance)) {
            if (distance < closestDistance) {
                closestDistance = distance;
                outHit->hitPoint = ray.origin + ray.diff * distance;
                outHit->distance = distance;
                outHit->object = this;
                isHit = true;
            }
        }
    }
    return isHit;
}
