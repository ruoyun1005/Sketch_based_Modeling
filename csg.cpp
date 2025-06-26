#include "csg.hpp"
#include <glm/gtc/epsilon.hpp>
#include<iostream>
using namespace std;
using namespace glm;



//生出half plane：把二維的輪廓線
vector<Plane> silhouettePlanes (const vector<Point>& contour,ViewPlane view){
    vector<Plane> planes;
    planes.clear();
    int N = contour.size();
    for (int i = 0; i < N-1; i++){
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
            n = vec3(0, - u.z, u.y);
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
static vector<vec3> stitchEdgesIntoLoop(vector<pair<vec3,vec3>>& cutSegs) {
    vector<vec3> loop;
    if (cutSegs.empty()) {
        cout << "cutSegs is empty" << std::endl;
        return loop;
    }

    const float eps = 1e-6f;
    // epsilon 比對
    auto same = [&](const vec3 &a, const vec3 &b){
        return glm::all(glm::epsilonEqual(a, b, eps));
    };

    // 先取第一條邊，當作起點→下一個點
    vec3 start = cutSegs[0].first;
    vec3 curr  = cutSegs[0].second;
    loop.push_back(start);
    loop.push_back(curr);

    // 把這條邊從列表中移除
    cutSegs.erase(cutSegs.begin());

    // 不斷找下一條以 curr 為端點的邊
    while (!cutSegs.empty() && !same(curr, start)) {
        bool advanced = false;
        for (auto it = cutSegs.begin(); it != cutSegs.end(); ++it) {
            if (same(it->first, curr)) {
                curr = it->second;
                loop.push_back(curr);
                cutSegs.erase(it);
                advanced = true;
                break;
            }
            else if (same(it->second, curr)) {
                curr = it->first;
                loop.push_back(curr);
                cutSegs.erase(it);
                advanced = true;
                break;
            }
        }
        if (!advanced) {
            // 如果找不到相連的邊，就跳出（理論上不應該發生）
            break;
        }
    }

    return loop;
}

//以一個平面去裁切一個多面體
Polyhedron clipByPlane(const Polyhedron& P, const Plane& pl){
    Polyhedron out;
    vector<pair<vec3,vec3>> cutSegs;

    for(auto& face : P.faces){
        vector<vec3> poly = { P.verts[face[0]], P.verts[face[1]], P.verts[face[2]] };// 取出原三角形的 3 個點

        vector<vec3> input = poly;
        vector<vec3> output;
        int N = (int)input.size();
        for (int i = 0; i < N; i++) {
        vec3 a = input[i];
        vec3 b = input[(i+1)%N];
        bool inA = isInside(a,pl);
        bool inB = isInside(b,pl);
        if (inA != inB) {
            vec3 I = intersectPlane(a,b,pl);
            vec3 J = I;
            if (inA && !inB)     output.push_back(I);
            else if (!inA && inB){
                output.push_back(I);
                output.push_back(b);
            }
            cutSegs.emplace_back(I,J);
        }
        else if (inA && inB) {
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
    auto loop = stitchEdgesIntoLoop(cutSegs);
    if(loop.size()>=3){
        // 計算質心
        glm::vec3 center(0.0f);
        for(auto &p: loop) center += p;
        center /= (float)loop.size();
        // 扇形三角化
        for(size_t i=1;i+1<loop.size();++i){
            int b = (int)out.verts.size();
            out.verts.push_back(center);
            out.verts.push_back(loop[i]);
            out.verts.push_back(loop[i+1]);
            out.faces.push_back({b,b+1,b+2});
        }
    }
    return out;
}

Polyhedron makeBoundingCube(float R) {
    Polyhedron P;
    // 8 個頂點
    vector<vec3> V = {
        { R,  R,  R}, {-R,  R,  R}, {-R, -R,  R}, { R, -R,  R},
        { R,  R, -R}, {-R,  R, -R}, {-R, -R, -R}, { R, -R, -R},
    };
    P.verts = V;
    // 六個面，每面拆兩個三角形
    int F[6][4] = {
      {0,1,2,3}, {4,7,6,5},
      {0,4,5,1}, {1,5,6,2},
      {2,6,7,3}, {3,7,4,0}
    };
    for(int i=0;i<6;i++){
      auto &q = F[i];
      P.faces.push_back({q[0],q[1],q[2]});
      P.faces.push_back({q[0],q[2],q[3]});
    }
    return P;
}
Polyhedron makePolyFromTriangles(const vector<vec3>& triVerts) {
    Polyhedron P;

    // 輸入長度必須是 3 的倍數
    if (triVerts.size() % 3 != 0) {
        cerr << "[makePolyFromTriangles] triVerts.size() = "
             << triVerts.size()
             << " 不是 3 的倍數！\n";
        return P;
    }

    size_t triCount = triVerts.size() / 3;
    P.verts.reserve(triVerts.size());
    P.faces.reserve(triCount);

    for (size_t i = 0; i < triCount; ++i) {
        // push 三個頂點
        int base = static_cast<int>(P.verts.size());
        P.verts.push_back(triVerts[3*i + 0]);
        P.verts.push_back(triVerts[3*i + 1]);
        P.verts.push_back(triVerts[3*i + 2]);
        // 對應一個面
        P.faces.push_back({ base, base+1, base+2 });
    }

    return P;
}
// helpers.hpp
Polyhedron makePolyFromSolid(const Solid& s) {
    Polyhedron P;
    P.verts.reserve(s.verts.size());
    P.faces.reserve(s.faces.size());
    // 1) 复制顶点
    for (auto& v : s.verts) {
        P.verts.push_back(v);
    }
    // 2) 复制面
    for (auto& f : s.faces) {
        P.faces.push_back(f);
    }
    return P;
}

