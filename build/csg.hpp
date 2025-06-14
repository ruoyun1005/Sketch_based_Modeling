#pragma once
#include <vector>
#include "cone_builder.hpp"

using namespace std;
using namespace glm;

//定義布林運算列舉
enum CSG_op { INTERSECT, UNION, SUBTRACT};

// hlaf plane nx + d >=0
struct Plane {
    vec3 n;
    float d;
};

//多面體 頂點 + 三角形面
struct Polyhedron {
    vector<vec3> verts;
    vector<array<int, 3>> faces;//每個 face 用三個索引指向 verts
};

//從輪廓線生出 half plane
enum class ViewPlane { XY, XZ, YZ };
vector<Plane> silhouettePlanes (
    const vector<Point>& contour,
    ViewPlane view
);

//用平面裁多面體
Polyhedron clipByPlane(const Polyhedron& P, const Plane& pl);

//把原本 buildCylinder 回傳的 vertices 轉成 Polyhedron
Polyhedron makePolyFromTriangles(
    const vector<vec3>& triVerts);
