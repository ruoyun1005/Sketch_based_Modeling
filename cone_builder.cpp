#include <glm/glm.hpp>
#include "cone_builder.hpp"

using namespace std;
using namespace glm;

// 生成柱體(xy平面延伸)
vector<vec3> buildCone_xy(const vector<Point>&contour, float height){
    vector<vec3> vertices;
    
    for (size_t i = 0; i < contour.size(); i++){
        
        //2D -> 3D
        vec3 bottom_point_1 = vec3(contour[i].x, contour[i].y, 0.0f);
        vec3 top_point_1 = vec3(contour[i].x, contour[i].y, height);
        
        size_t j = (i+1) % contour.size();
        vec3 bottom_point_2 = vec3(contour[j].x, contour[j].y, 0.0f);
        vec3 top_point_2 = vec3(contour[j].x, contour[j].y, height);
        
        vertices.push_back(top_point_1);
        vertices.push_back(bottom_point_1); 
        vertices.push_back(bottom_point_2);
        
        vertices.push_back(top_point_1); 
        vertices.push_back(top_point_2); 
        vertices.push_back(bottom_point_2);  
    }
    //柱體底面
    vec3 center_bottom(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_bottom += vec3(p.x, p.y, 0.0f);
    }
    center_bottom /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(contour[i].x, contour[i].y, 0.0f);
        vec3 p2 = vec3(contour[j].x, contour[j].y, 0.0f);

        vertices.push_back(center_bottom);
        vertices.push_back(p1);
        vertices.push_back(p2);
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

        vertices.push_back(center_top);
        vertices.push_back(p1);
        vertices.push_back(p2);
    }
    return vertices;
}

// 生成柱體(xz平面延伸)
vector<vec3> buildCone_xz(const vector<Point>&contour, float height){
    vector<vec3> vertices;
    
    for (size_t i = 0; i < contour.size(); i++){
        
        //2D -> 3D
        vec3 bottom_point_1 = vec3(contour[i].x, 0.0f, contour[i].y);
        vec3 top_point_1 = vec3(contour[i].x, height, contour[i].y);
        
        size_t j = (i+1) % contour.size();
        vec3 bottom_point_2 = vec3(contour[j].x, 0.0f, contour[j].y);
        vec3 top_point_2 = vec3(contour[j].x, height, contour[j].y);
        
        vertices.push_back(top_point_1);
        vertices.push_back(bottom_point_1); 
        vertices.push_back(bottom_point_2);
        
        vertices.push_back(top_point_1); 
        vertices.push_back(top_point_2); 
        vertices.push_back(bottom_point_2);

        
    }
    //柱體底面
    vec3 center_bottom(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_bottom += vec3(p.x, 0.0f, p.y);
    }
    center_bottom /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(contour[i].x, 0.0f, contour[i].y);
        vec3 p2 = vec3(contour[j].x, 0.0f, contour[j].y);

        vertices.push_back(center_bottom);
        vertices.push_back(p1);
        vertices.push_back(p2);
    }
    // 柱體頂面
    vec3 center_top(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_top += vec3(p.x, p.y, height);
    }
    center_top /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(contour[i].x, height, contour[i].y);
        vec3 p2 = vec3(contour[j].x, height, contour[j].y);

        vertices.push_back(center_top);
        vertices.push_back(p1);
        vertices.push_back(p2);
    }
    return vertices;
}

// 生成柱體(yz平面延伸)
vector<vec3> buildCone_yz(const vector<Point>&contour, float height){
    vector<vec3> vertices;
    
    for (size_t i = 0; i < contour.size(); i++){
        
        //2D -> 3D
        vec3 bottom_point_1 = vec3(0.0f, contour[i].x, contour[i].y);
        vec3 top_point_1 = vec3(height, contour[i].x, contour[i].y);
        
        size_t j = (i+1) % contour.size();
        vec3 bottom_point_2 = vec3(0.0f, contour[j].x, contour[j].y);
        vec3 top_point_2 = vec3(height, contour[j].x, contour[j].y);
        
        vertices.push_back(top_point_1);
        vertices.push_back(bottom_point_1); 
        vertices.push_back(bottom_point_2);
        
        vertices.push_back(top_point_1); 
        vertices.push_back(top_point_2); 
        vertices.push_back(bottom_point_2);

        
    }
    //柱體底面
    vec3 center_bottom(0.0f, 0.0f, 0.0f);
    for(const auto& p : contour){
        center_bottom += vec3(0.0f, p.x, p.y);
    }
    center_bottom /= static_cast<float>(contour.size());

    for(size_t i = 0; i < contour.size(); i++){
        size_t j = (i+1) % contour.size();
        vec3 p1 = vec3(0.0f, contour[i].x, contour[i].y);
        vec3 p2 = vec3(0.0f, contour[j].x, contour[j].y);

        vertices.push_back(center_bottom);
        vertices.push_back(p1);
        vertices.push_back(p2);
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

        vertices.push_back(center_top);
        vertices.push_back(p1);
        vertices.push_back(p2);
    }
    return vertices;
}