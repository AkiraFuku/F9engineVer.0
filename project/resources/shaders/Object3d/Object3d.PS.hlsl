#include "Object3d.hlsli"
struct Material
{
    float4 Color;
    int enableLighting;
    int diffuseType; // 0:Lambert, 1:Half-Lambert
    int specularType; // 0:None, 1:Phong, 2:BlinnPhong
    float4x4 uvTransform; // UV変換行列
    float shininess;
};
struct DirectionalLight
{
    float4 color; //ライトの色
    float3 direction; //ライトの向き
    float intensity; // 明るさ


};
struct PointLight
{
    float4 color; //ライトの色
    float3 position; //ライトの向き
    float intensity; // 明るさ
    float radius;
    float decay;
    float2 padding;
};
struct SpotLight
{
    float4 color; //ライトの色
    float3 position; //ライトの向き
    float intensity; // 明るさ
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
    float padding;
   
};
struct AreaLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 right; // 幅の半分 * 回転
    float decay;
    float3 up; // 高さの半分 * 回転
    float range;
};
struct Camera
{
    float3 worldPosition;
};
struct LightCounts
{
    int numDirectionalLights;
    int numPointLights;
    int numSpotLights;
    int numAreaLights;
};
ConstantBuffer<Camera> gCamera : register(b2);

ConstantBuffer<Material> gMaterial : register(b0);

StructuredBuffer<DirectionalLight> gDirectionalLights : register(t1);
StructuredBuffer<PointLight> gPointLights : register(t2);
StructuredBuffer<SpotLight> gSpotLights : register(t3);
StructuredBuffer<AreaLight> gAreaLights : register(t4);
// ★追加: カウント用定数バッファ (CBV: b3)
ConstantBuffer<LightCounts> gLightCounts : register(b3);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);


// -------------------------------------------------------------
// ライティング計算用関数
// N: 法線, L: ライトへの方向, V: カメラへの方向, lightColor: ライトの色, intensity: 強度
// -------------------------------------------------------------
float3 CalculateLight(float3 N, float3 L, float3 V, float3 lightColor, float intensity)
{
    // 1. 拡散反射 (Diffuse)
    float NdotL = dot(N, L);
    float cos = saturate(NdotL);
    if (gMaterial.diffuseType == 1) // Half-Lambert
    {
        cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    float3 diffuse = gMaterial.Color.rgb * lightColor * cos * intensity;

    // 2. 鏡面反射 (Specular)
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float specularPow = 0.0f;

    if (gMaterial.specularType == 1) // Phong
    {
        float3 R = reflect(-L, N);
        float RdotV = dot(R, V);
        specularPow = pow(saturate(RdotV), gMaterial.shininess);
    }
    else if (gMaterial.specularType == 2) // Blinn-Phong
    {
        float3 H = normalize(L + V);
        float NdotH = dot(N, H);
        specularPow = pow(saturate(NdotH), gMaterial.shininess);
    }
    
    if (gMaterial.specularType != 0)
    {
        specular = lightColor * intensity * specularPow;
    }

    return diffuse + specular;
}


float3 ClosestPointOnRect(float3 P, float3 lightPos, float3 lightRight, float3 lightUp)
{
    float3 d = P - lightPos;
    
    // 各軸への射影
    float u = dot(d, lightRight) / dot(lightRight, lightRight); // 正規化されていない軸対応
    float v = dot(d, lightUp) / dot(lightUp, lightUp);

    // -1 ~ 1 の範囲にクランプ (right/upは半分の長さを持っているので、係数は-1~1)
    u = clamp(u, -1.0f, 1.0f);
    v = clamp(v, -1.0f, 1.0f);

    return lightPos + lightRight * u + lightUp * v;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float4 textureColor = gTexture.Sample(gSampler, input.texCoord); // UV変換はVSで行っている前提、または必要ならここで計算

    // ライティングが無効ならそのまま返す
    if (gMaterial.enableLighting == 0)
    {
        
        output.color = gMaterial.Color * textureColor;
         // アルファテスト
        if (output.color.a < 0.01f)
        {
            discard;
        }
        return output;
    }

    float3 N = normalize(input.normal);
    float3 V = normalize(gCamera.worldPosition - input.worldPosition);
    
    float3 finalLighting = float3(0.0f, 0.0f, 0.0f);

    
    // Directional Light
    for (int i = 0; i < gLightCounts.numDirectionalLights; ++i)
    {
        // gDirectionalLights[i] でアクセス
        float3 L_dir = normalize(-gDirectionalLights[i].direction);
        finalLighting += CalculateLight(N, L_dir, V, gDirectionalLights[i].color.rgb, gDirectionalLights[i].intensity);
    }
    
    //ポイントライト
    for (int j = 0; j < gLightCounts.numPointLights; ++j)
    {
        // 強度が0以下のライトは計算スキップ
        if (gPointLights[j].intensity <= 0.0f)
        {
            continue;
        }
        float3 directionToPointLight = gPointLights[j].position - input.worldPosition;
    // 距離による減衰は計算せず、正規化して方向だけ使う
        float3 L_point = normalize(directionToPointLight);
        float distance = length(directionToPointLight);
        float factor = pow(saturate(-distance / gPointLights[j].radius + 1.0f), gPointLights[j].decay);
        finalLighting += CalculateLight(N, L_point, V, gPointLights[j].color.rgb, gPointLights[j].intensity * factor);
    }
    //スポットライト
    for (int k = 0; k < gLightCounts.numSpotLights; ++k)
    {
        // 強度が0以下のライトは計算スキップ
        if (gSpotLights[k].intensity <= 0.0f)
        {
            continue;
        }
         
 // 1. 光源への方向ベクトルと距離を計算
        float3 directionToSpotLight = gSpotLights[k].position - input.worldPosition;
        float distanceSpot = length(directionToSpotLight);
        float3 L_spot = normalize(directionToSpotLight); // 光源方向 (単位ベクトル)

    // 2. 距離による減衰 (Falloff)
    // PointLightと同じく、指定距離(distance)で強度が0になるよう計算
        float distFactor = pow(saturate(-distanceSpot / gSpotLights[k].distance + 1.0f), gSpotLights[k].decay);

    // 3. 角度による減衰 (Cone Falloff)
        float cosAngle = dot(L_spot, gSpotLights[k].direction);

        float cosDiff = gSpotLights[k].cosFalloffStart - gSpotLights[k].cosAngle;

        float range = max(cosDiff, 0.0001f);
        float angleFactor = saturate((cosAngle - gSpotLights[k].cosAngle) / range);
    
        finalLighting += CalculateLight(N, L_spot, V, gSpotLights[k].color.rgb, gSpotLights[k].intensity * distFactor * angleFactor);
    }
    for (int a = 0; a < gLightCounts.numAreaLights; ++a)
    {
        // 強度が0以下のライトは計算スキップ
        if (gAreaLights[a].intensity <= 0.0f)
        {
            continue;
        }
        // 1. エリアライトの矩形上で、現在のピクセル位置に最も近い点を算出
        // 定義済みの ClosestPointOnRect 関数を使用
        float3 closestPoint = ClosestPointOnRect(input.worldPosition, gAreaLights[a].position, gAreaLights[a].right, gAreaLights[a].up);

        // 2. 光源への方向ベクトルと距離を計算
        // 通常のポイントライトと異なり、中心位置ではなく「最も近い点」へのベクトルを使用する
        float3 directionToAreaLight = closestPoint - input.worldPosition;
        float distanceArea = length(directionToAreaLight);
        float3 L_area = normalize(directionToAreaLight);

        // 3. 距離による減衰 (Falloff)
        // PointLight等と同様の減衰計算 (radiusの代わりにrangeを使用)
        float factor = pow(saturate(-distanceArea / gAreaLights[a].range + 1.0f), gAreaLights[a].decay);
        
        // 4. ライティング計算
        // 算出した L_area を使って、共通の CalculateLight 関数で拡散反射・鏡面反射を計算
        finalLighting += CalculateLight(N, L_area, V, gAreaLights[a].color.rgb, gAreaLights[a].intensity * factor);
    }
    output.color.rgb = finalLighting * textureColor.rgb;
    output.color.a = gMaterial.Color.a * textureColor.a;
    
    // アルファテスト
    if (output.color.a < 0.01f)
    {
        discard;
    }
    return output;
}