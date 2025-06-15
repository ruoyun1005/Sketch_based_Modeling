#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include "cylinder_builder.hpp"
#include "csg.hpp"

using namespace std;
using namespace glm;

// 1) 生成柱體(xy平面延伸)
//---------------------------------------
Solid buildCylinderXY(const vector<Point>&contour, float height){
    Solid S;
    int N = (int)contour.size();
    
    for (int i = 0; i < N; i++){
        
        //2D -> 3D
        vec3 bottom_point_1 = vec3(contour[i].x, contour[i].y, -height);
        vec3 top_point_1 = vec3(contour[i].x, contour[i].y, height);
        
        int j = (i+1) % contour.size();
        vec3 bottom_point_2 = vec3(contour[j].x, contour[j].y, -height);
        vec3 top_point_2 = vec3(contour[j].x, contour[j].y, height);
        
        int idx = (int)S.verts.size();
        S.verts.push_back(top_point_1);
        S.verts.push_back(bottom_point_1); 
        S.verts.push_back(bottom_point_2);
        
        S.verts.push_back(top_point_1); 
        S.verts.push_back(top_point_2); 
        S.verts.push_back(bottom_point_2); 
        
        S.faces.push_back({idx+0,idx+1, idx+2});
        S.faces.push_back({idx+3,idx+4, idx+5});
    }
    //柱體底面
    vec3 center_bottom(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_bottom += vec3(p.x, p.y, 0.0f);
    }
    center_bottom /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(contour[i].x, contour[i].y, -height);
        vec3 p2 = vec3(contour[j].x, contour[j].y, -height);

        int idx = (int)S.verts.size();
        S.verts.push_back(center_bottom);
        S.verts.push_back(p1);
        S.verts.push_back(p2);
        S.faces.push_back({idx+0, idx+1, idx+2});
    }
    // 柱體頂面
    vec3 center_top(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_top += vec3(p.x, p.y, height);
    }
    center_top /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(contour[i].x, contour[i].y, height);
        vec3 p2 = vec3(contour[j].x, contour[j].y, height);

        int idx = (int)S.verts.size();
        S.verts.push_back(center_top);
        S.verts.push_back(p1);
        S.verts.push_back(p2);
        S.faces.push_back({idx+0, idx+1, idx+2});
    }
    // 產生半平面
    auto side = silhouettePlanes(contour, ViewPlane:: XY);
    S.planes.insert(S.planes.end(), side.begin(), side.end());
    // 底面 z=0, 內側往上：  0·x+0·y+(−1)·z + h = 0
    //S.planes.push_back({{0,0,-1}, height});
    // 頂面 z=H, 內側往下： 0·x+0·y+1·z + (−h) = 0
    //S.planes.push_back({{0,0, 1}, -height});
    return S;
}

// 生成柱體(xz平面延伸)
Solid buildCylinderXZ(const vector<Point>&contour, float height){
    Solid S;
    int N = (int)contour.size();
    
    for (int i = 0; i < contour.size(); i++){
        
        //2D -> 3D
        vec3 bottom_point_1 = vec3(contour[i].x, -height, contour[i].y);
        vec3 top_point_1 = vec3(contour[i].x, height, contour[i].y);
        
        int j = (i+1) % contour.size();
        vec3 bottom_point_2 = vec3(contour[j].x, -height, contour[j].y);
        vec3 top_point_2 = vec3(contour[j].x, height, contour[j].y);
        
        int idx = (int)S.verts.size();
        S.verts.push_back(top_point_1);
        S.verts.push_back(bottom_point_1); 
        S.verts.push_back(bottom_point_2);
        
        S.verts.push_back(top_point_1); 
        S.verts.push_back(top_point_2); 
        S.verts.push_back(bottom_point_2);

        S.faces.push_back({idx+0,idx+1, idx+2});
        S.faces.push_back({idx+3,idx+4, idx+5});

        
    }
    //柱體底面
    vec3 center_bottom(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_bottom += vec3(p.x, -height, p.y);
    }
    center_bottom /= static_cast<float>(contour.size());

    for(int i = 0; i < N; i++){
        int j = (i+1) % N;
        vec3 p1 = vec3(contour[i].x, -height, contour[i].y);
        vec3 p2 = vec3(contour[j].x, -height, contour[j].y);

        int idx = (int)S.verts.size();
        S.verts.push_back(center_bottom);
        S.verts.push_back(p1);
        S.verts.push_back(p2);
        S.faces.push_back({idx+0, idx+1, idx+2});

    }
    // 柱體頂面
    vec3 center_top(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_top += vec3(p.x, p.y, height);
    }
    center_top /= static_cast<float>(contour.size());

    for(int i = 0; i < N; i++){
        int j = (i+1) % N;
        vec3 p1 = vec3(contour[i].x, height, contour[i].y);
        vec3 p2 = vec3(contour[j].x, height, contour[j].y);

        int idx = (int)S.verts.size();
        S.verts.push_back(center_top);
        S.verts.push_back(p1);
        S.verts.push_back(p2);
        S.faces.push_back({idx+0, idx+1, idx+2});
    }
    // 產生半平面
    auto side = silhouettePlanes(contour, ViewPlane:: XZ);
    S.planes.insert(S.planes.end(), side.begin(), side.end());
    // 底面 z=0, 內側往上：  0·x+0·y+(−1)·z + h = 0
    //S.planes.push_back({{0,0,-1}, height});
    // 頂面 z=H, 內側往下： 0·x+0·y+1·z + (−h) = 0
    //S.planes.push_back({{0,0, 1}, -height});
    return S;
   
}

// 生成柱體(yz平面延伸)
Solid buildCylinderYZ(const vector<Point>&contour, float height){
    Solid S;
    int N = (int)contour.size();
    
    for (int i = 0; i < N; i++){
        
        //2D -> 3D
        vec3 bottom_point_1 = vec3( -height, contour[i].x, contour[i].y);
        vec3 top_point_1 = vec3(height, contour[i].x, contour[i].y);
        
        int j = (i+1) % contour.size();
        vec3 bottom_point_2 = vec3( -height, contour[j].x, contour[j].y);
        vec3 top_point_2 = vec3(height, contour[j].x, contour[j].y);
        
        int idx = (int)S.verts.size();
        S.verts.push_back(top_point_1);
        S.verts.push_back(bottom_point_1); 
        S.verts.push_back(bottom_point_2);
        
        S.verts.push_back(top_point_1); 
        S.verts.push_back(top_point_2); 
        S.verts.push_back(bottom_point_2);
        
        S.faces.push_back({idx+0, idx+1, idx+2});
        S.faces.push_back({idx+3, idx+4, idx+5});

        
    }
    //柱體底面
    vec3 center_bottom(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_bottom += vec3( -height, p.x, p.y);
    }
    center_bottom /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3( -height, contour[i].x, contour[i].y);
        vec3 p2 = vec3( -height, contour[j].x, contour[j].y);

        int idx = (int)S.verts.size();
        S.verts.push_back(center_bottom);
        S.verts.push_back(p1);
        S.verts.push_back(p2);
        S.faces.push_back({idx+0, idx+1, idx+2});
        
    }
    // 柱體頂面
    vec3 center_top(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_top += vec3(height, p.x, p.y);
    }
    center_top /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(height, contour[i].x, contour[i].y);
        vec3 p2 = vec3(height, contour[j].x, contour[j].y);

        int idx = (int)S.verts.size();
        S.verts.push_back(center_top);
        S.verts.push_back(p1);
        S.verts.push_back(p2);
        S.faces.push_back({idx+0, idx+1, idx+2});
    }
    // 產生半平面
    auto side = silhouettePlanes(contour, ViewPlane:: YZ);
    S.planes.insert(S.planes.end(), side.begin(), side.end());
    // 底面 z=0, 內側往上：  0·x+0·y+(−1)·z + h = 0
    //S.planes.push_back({{0,0,-1}, height});
    // 頂面 z=H, 內側往下： 0·x+0·y+1·z + (−h) = 0
    //S.planes.push_back({{0,0, 1}, -height});
    
    return S;
}