#pragma once
#include <vector>
#include "cylinder_builder.hpp"

using namespace std;
using namespace glm;

//從輪廓線生出 half plane
enum class ViewPlane { XY, XZ, YZ };

//定義布林運算列舉
enum CSG_op { INTERSECT, UNION, DIFFERENCE};

//儲存輪廓、操作、哪個平面
struct Silhouette {
    ViewPlane view;        //在哪個平面上
    vector<Point> contour; //輪廓線
    CSG_op op;             //布林運算
};

//多面體 頂點 + 三角形面
struct Polyhedron {
    vector<vec3> verts;
    vector<array<int, 3>> faces;//每個 face 用三個索引指向 verts
};


vector<Plane> silhouettePlanes (
    const vector<Point>& contour,
    ViewPlane view
);

//用平面裁多面體
Polyhedron clipByPlane(const Polyhedron& P, const Plane& pl);

//把原本 buildCylinder 回傳的 vertices 轉成 Polyhedron
Polyhedron makePolyFromTriangles(
    const vector<vec3>& triVerts);

Polyhedron makeBoundingCube(float R);