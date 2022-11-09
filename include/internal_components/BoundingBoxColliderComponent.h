#pragma once

#include "BoundingBox.h"
#include "IColliderComponent.h"

namespace CollisionInternal {
bool intersectionWithRay(const Vector3f &point, const Vector3f &ray, const BoundingBox& box, Vector3f &intersection) {
    std::vector<Vector3f> intersections;
    auto vertices = box.GetVertices();
    auto triangles = box.GetTriangles();
    Vector3f pontos[12];

    for (int i = 0; i < 6; i++) {
        Vector3f v1{ std::vector<float>(vertices[triangles[i*6]], vertices[triangles[i*6]] + 3) };
        Vector3f v2 = vertices[3*triangles[i*6 + 1]];
        Vector3f v3 = vertices[3*triangles[i*6 + 2]];
        Vector3f v4 = vertices[3*triangles[i*6 + 3]];
        Vector3f v5 = vertices[3*triangles[i*6 + 4]];
        Vector3f v6 = vertices[3*triangles[i*6 + 5]];

        Vector3f normal = (v2 - v1).CrossProduct(v3 - v1).Normalized();

        Vector3f ponto = v1;

        float a = normal.x;
        float b = normal.y;
        float c = normal.z;
        float d = -(a * ponto.x + b * ponto.y + c * ponto.z);

        float t = -(d + a * point.x + b * point.y + c * point.z) / (a * ray.x + b * ray.y + c * ray.z);

        Vector3f inter = point + t * ray;


        if (isInsideTriangle(v1, v2, v3, inter)) {
            pontos[i] = inter;
            //intersec.push_back(&pontos[i], (int) (point - inter).Length());
        }
        normal = (v4 - v3).CrossProduct(v1 - v3).Normalized();

        ponto = v3;

        a = normal.x;
        b = normal.y;
        c = normal.z;
        d = -(a * ponto.x + b * ponto.y + c * ponto.z);

        t = -(d + a * point.x + b * point.y + c * point.z) / (a * ray.x + b * ray.y + c * ray.z);

        inter = point + t * ray;

        if (isInsideTriangle(v3, v4, v1, inter)) {
            pontos[2 * i] = inter;
            //intersec.add(&pontos[2 * i], (int) (point - inter).Length());
        }
    }

    if (!intersections.empty()) {
        //        intersection = *(intersec.get(0));
        return true;
    }
    intersection = Vector3f(9999999, 9999999, 9999999);

    return false;
}


bool BoundingBox::isInsideTriangle(const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, const Vector3f &intersection) {
    //Checa se a coordenada está limitada pelo triangulo
    float area = (v2 - v1).CrossProduct(v3 - v1).Length() * 0.5f;
    Vector3f vet1 = intersection - v1;
    Vector3f vet2 = v2 - v1;
    float area1 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = intersection - v1;
    vet2 = v3 - v1;
    float area2 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    vet1 = intersection - v3;
    vet2 = v2 - v3;
    float area3 = vet1.CrossProduct(vet2).Length() * 0.5f / area;

    //Condição para que o ponto esteja contido no triangulo
    if (area1 + area2 + area3 <= 1.001f)
        return true;

    return false;
}

}// namespace CollisionInternal

class CBoundingBoxColliderComponent : public IColliderComponent {
public:
    CBoundingBoxColliderComponent() = default;
    ~CBoundingBoxColliderComponent() override = default;

    [[nodiscard]] bool HasCollided() const override;

private:
    BoundingBox mBoundingBox;
};
