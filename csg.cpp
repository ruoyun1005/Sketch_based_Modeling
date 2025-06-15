#include "csg.hpp"
#include <glm/gtc/epsilon.hpp>
#include <iostream>

using namespace std;
using namespace glm;



//生出half plane：把二維的輪廓線
vector<Plane> silhouettePlanes(const vector<Point>& contour, ViewPlane view) {
    vector<Plane> planes;
    int N = contour.size();
    for (int i = 0; i < N; i++) {
        auto a = contour[i];
        auto b = contour[(i + 1) % N];
        vec3 u, p0, n;
        if (view == ViewPlane::XY) {
            u = vec3(b.x - a.x, b.y - a.y, 0);
            p0 = vec3(a.x, a.y, 0);
            n = vec3(u.y, -u.x, 0); // 反轉為順時針輪廓的外部法線
        } else if (view == ViewPlane::XZ) {
            u = vec3(b.x - a.x, 0, b.y - a.y);
            p0 = vec3(a.x, 0, a.y);
            n = vec3(u.z, 0, -u.x);
        } else if (view == ViewPlane::YZ) {
            u = vec3(0, b.x - a.x, b.y - a.y);
            p0 = vec3(0, a.x, a.y);
            n = vec3(0, u.z, -u.y);
        }
        if (length(u) < 1e-6f) continue; // 跳過零向量
        n = normalize(n);
        float d = -dot(n, p0);
        planes.push_back({n, d});
        std::cout << "[debug] Plane n=(" << n.x << "," << n.y << "," << n.z << ") d=" << d << "\n";
    }
    return planes;
}

//測試在平面的哪一側 ( let n.p d >=0 為內側 )
bool isInside(const vec3& p, const Plane& pl){
    return dot(pl.n, p) + pl.d >= -1e-6f;
}

// 求線段 p1→p2 與平面的交點( 這裡的 p1 p2 是從原本建構柱體的那些三角形片面來的，跟上面的pi pi+i不一樣 )
vec3 intersectPlane(const vec3& p1, const vec3& p2, const Plane& pl){
    float v1 = dot(pl.n, p1) + pl.d;
    float v2 = dot (pl.n, p2) + pl.d;
    float t = v1 / (v1 - v2);
    if (abs(v1 - v2) < 1e-6f) return p1;
    return p1 + t * (p2 - p1);
}

//以一個平面去裁切一個多面體
Polyhedron clipByPlane(const Polyhedron& P, const Plane& pl) {
    Polyhedron out;
    out.verts = P.verts;

    vector<bool> inside(P.verts.size());
    for (size_t i = 0; i < P.verts.size(); ++i) {
        inside[i] = isInside(P.verts[i], pl);
    }

    for (const auto& face : P.faces) {
        vector<int> newFace;
        for (size_t i = 0; i < 3; ++i) {
            int idxA = face[i];
            int idxB = face[(i + 1) % 3];
            bool inA = inside[idxA], inB = inside[idxB];

            if (inA) newFace.push_back(idxA);
            if (inA != inB) {
                vec3 intersect = intersectPlane(P.verts[idxA], P.verts[idxB], pl);
                if (isnan(intersect.x) || isnan(intersect.y) || isnan(intersect.z)) continue;
                out.verts.push_back(intersect);
                newFace.push_back(out.verts.size() - 1);
            }
        }

        if (newFace.size() >= 3) {
            // 對多邊形進行三角化
            for (size_t i = 1; i + 1 < newFace.size(); ++i) {
                out.faces.push_back({newFace[0], newFace[i], newFace[i + 1]});
            }
        }
    }

    if (out.faces.empty()) {
        std::cout << "[debug] No valid faces after clip\n";
        return out;
    }

    // 優化頂點
    vector<vec3> usedVerts;
    vector<int> vertMap(out.verts.size(), -1);
    for (const auto& face : out.faces) {
        for (int idx : face) {
            if (vertMap[idx] == -1) {
                vertMap[idx] = usedVerts.size();
                usedVerts.push_back(out.verts[idx]);
            }
        }
    }
    out.verts = usedVerts;
    for (auto& face : out.faces) {
        for (int& idx : face) {
            idx = vertMap[idx];
        }
    }

    std::cout << "[debug] Clip result: " << out.verts.size() << " verts, " << out.faces.size() << " faces\n";
    return out;
}

Polyhedron makePolyFromTriangles(const std::vector<glm::vec3>& triVerts) {
    Polyhedron P;
    int triCount = triVerts.size() / 3;
    if (triVerts.size() % 3 != 0) {
        std::cout << "[debug] Invalid triVerts size: " << triVerts.size() << "\n";
        return P;
    }
    for(int i = 0; i < triCount; ++i) {
        int b = i * 3;
        // 三角形 (b, b+1, b+2)
        P.verts.push_back(triVerts[b + 0]);
        P.verts.push_back(triVerts[b + 1]);
        P.verts.push_back(triVerts[b + 2]);
        P.faces.push_back({ b + 0, b + 1, b + 2 });
    }
    return P;
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