#ifndef CONE_BUILDER_HPP
#define CONE_BUILDER_HPP

//#pragma once //防止重複引入
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

using namespace std;
using namespace glm;

struct Point{
    float x, y;
};

// hlaf plane nx + d >=0
struct Plane {
    vec3 n;
    float d;
};

struct Solid{
    vector<vec3> verts;
    vector<array<int, 3>> faces;
    vector<Plane> planes; 
};

Solid buildCylinderXY(const vector<Point>&contour, float height);// xy平面 左上角視窗
Solid buildCylinderXZ(const vector<Point>&contour, float height);// xz平面 右下角視窗
Solid buildCylinderYZ(const vector<Point>&contour, float height);// yz平面 左下角視窗

#endif