#ifndef CONE_BUILDER_HPP
#define CONE_BUILDER_HPP

//#pragma once //防止重複引入
#include <vector>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

struct Point{
    float x, y;
};

vector<vec3> buildCone_xy(const vector<Point>&contour, float height);// xy平面 左上角視窗
vector<vec3> buildCone_xz(const vector<Point>&contour, float height);// xz平面 右下角視窗
vector<vec3> buildCone_yz(const vector<Point>&contour, float height);// yz平面 左下角視窗

#endif