#include "csg.hpp"
#include <glm/gtc/epsilon.hpp>

using namespace std;
using namespace glm;

vector<Plane> planes;

//生出half plane：把二維的輪廓線
vector<Plane> silhouettePlanes (const vector<Point>& contour,ViewPlane view){
    int N = contour.size();
    for (int i = 0; i < N; i++){
        auto a = contour[i];// p_i
        auto b = contour[(i+1) % N];// p_i+1
        vec3 u, p0, n;//u 是線段（pi pi+1)的向量   n 是法向向量  p0 另為起始點 p1 in R3( p_i in R2 )
        if (view == ViewPlane::XY){
            u = vec3(b.x - a.x, b.y - a.y, 0);
            p0 = vec3(a.x, a.y, 0);
            n = vec3(-u.y, u.x, 0);
        }
        if(view == ViewPlane::XZ){
            u = vec3(b.x - a.x, 0, b.y - a.y);
            p0 = vec3(a.x, 0, a.y);
            n = vec3(-u.z, 0, u.x);
        }
        if(view == ViewPlane::YZ){
            u = vec3(0, b.x - a.x, b.y - a.y);
            p0 = vec3(0, a.x, a.y);
            n = vec3(0, -u.z, u.x);
        }
        n = normalize(n);
        float d = - dot(n, p0);
        planes.push_back({n,d});
    }
    return planes;
}

//測試在平面的哪一側 ( let n.p d >=0 為內側 )
bool isInside(const vec3& p, const Plane& pl){
    return dot(pl.n, p) + pl.d >=0.0f;
}

// 求線段 p1→p2 與平面的交點( 這裡的 p1 p2 是從原本建構柱體的那些三角形片面來的，跟上面的pi pi+i不一樣 )
vec3 intersectPlane(const vec3& p1, const vec3& p2, const Plane& pl){
    float v1 = dot(pl.n, p1) + pl.d;
    float v2 = dot (pl.n, p2) + pl.d;
    float t = v1 / (v1 - v2);

    return p1 + t * (p2 - p1);
}

//以一個平面去裁切一個多面體
Polyhedron clipByPlane(const Polyhedron& P, const Plane& pl){
    Polyhedron out;

    for(auto& face : P.faces){
        vector<vec3> poly = { P.verts[face[0]], P.verts[face[1]], P.verts[face[2]] };// 取出原三角形的 3 個點

        vector<vec3> input = poly;
        vector<vec3> output;
        int N = (int)input.size();
        for (int i = 0; i < N; i++){
            vec3 a = input[i];
            vec3 b = input[(i+1) % N];
            bool inA = isInside(a, pl);
            bool inB = isInside(b, pl);

            if(inA && inB){
                output.push_back(b);
            }
            if(inA && !inB){
                output.push_back(intersectPlane(a, b, pl));
            }
            if(!inA && inB){
                output.push_back(intersectPlane(a, b, pl));
                output.push_back(b);
            }
        }
        //如果裁剪後頂點數 < 3，就忽略
        if(output.size() < 3) continue;

        //三角化
        for(size_t i = 1; i+1 < output.size(); i++){
            int base = (int)out.verts.size();

            out.verts.push_back( output[0] );
            out.verts.push_back( output[i] );
            out.verts.push_back( output[i+1] );
            out.faces.push_back({ base+0, base+1, base+2 });
        }
    }
    return out;
}