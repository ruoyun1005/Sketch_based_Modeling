#include<iostream>
#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h> 
#include <OpenGL/glu.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "cylinder_builder.hpp"
#include "csg.hpp" 

// 定義 Point 的 != 運算子
inline bool operator!=(const Point& a, const Point& b) {
    return a.x != b.x || a.y != b.y;
}

// #include "imgui.h"
// #include "imgui_impl_glfw.h"
// #include "imgui_impl_opengl3.h"

GLuint vao, vbo;

using namespace glm;
using namespace std;

//記錄上次點的是哪一個視圖
enum  ViewID {
    View_front = 0,
    View_top = 1,
    View_left = 2,
};
//每一次點擊就發生一次事件：記錄在哪一張畫布話畫點的座標
struct SnapEvent {
    ViewID view;//在哪個圖
    float a, b;// 座標
};
vector<SnapEvent> snapEvents;
vector<Point> linePoints_front;
vector<Point> linePoints_left;
vector<Point> linePoints_top;
vector<vec3> Triangle_vertices_front;
vector<vec3> Triangle_vertices_top;
vector<vec3> Triangle_vertices_left;
vector<vec3> allvertices;


bool showSnapLines = false;
ViewID lastview = View_top;
float snapX = 0.0f, snapY = 0.0f, snapZ = 0.0f;// 記錄上記錄上次點擊時的座標分量


float theta = 0.0f;
float phi = 0.0f;
mat4x4 transformMat = mat4x4(1);

GLsizei vertsXY = 0, vertsXZ = 0, vertsYZ = 0;

float radius = 1.0f;
vec3 cameraPos(3.0f, 3.0f, radius);
vec3 cameraUp(0.0f, 0.0f, 1.0f);

bool shouldFill = false;

CSG_op currentOP = CSG_op::INTERSECT; // 預設布林運算為交集

void dump_plane(const vector<Plane>& pls, const string& name){
    cout << name << " ( " << pls.size() <<" planes):\n";
    for(size_t i = 0; i< pls.size(); i++){
        const auto& pl = pls[i];
        cout << "  [" << i << "] n=(" << pl.n.x << ", " << pl.n.y <<"," << pl.n.z << "), d=" << pl.d << "\n";

    }
}

// --------------------------------------------------
// 滑鼠按下記錄點
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        double xpos, ypos;// 取得滑鼠游標
        glfwGetCursorPos(window, &xpos, &ypos);
        
        ypos = height - ypos;

        int halfW = width / 2;
        int halfH = height / 2;

        float localX, localY;
        
        float nx = (float)( xpos / (width/2) * 2.0 - 1.0 );   // 依照不同區域會改
        float ny = (float)( ypos / (height/2) * 2.0 - 1.0 );
        const float step = 0.1f;// 網格間隔

        if (xpos < halfW && ypos < halfH) {
            // 左下畫布 (YZ)
            localX = xpos / halfW * 2.0f - 1.0f;
            localY = ypos / halfH * 2.0f - 1.0f;
            
            
            float snappedY = round(localX / step) * step;
            float snappedZ = round(localY / step) * step;       
            linePoints_front.push_back({snappedY, snappedZ});
            lastview = View_front;
            snapZ = snappedZ;
            snapY = snappedY;
            snapEvents.push_back({View_front, snapY, snapZ});
        } 
        else if (xpos < halfW && ypos >= halfH) {
            // 左上畫布 (XY)
            localX = xpos / halfW * 2.0f - 1.0f;
            localY = (ypos - halfH) / halfH * 2.0f - 1.0f;

            float step = 0.1f;
            float snappedX = round(localX / step) * step;
            float snappedY = round(localY / step) * step;       
            linePoints_top.push_back({snappedX, snappedY});
            lastview = View_top;
            snapX = snappedX;
            snapY = snappedY;
            snapEvents.push_back({View_top, snapX, snapY});
        } 
        else if (xpos >= halfW && ypos < halfH) {
            // 右下畫布 (XZ)
            localX = (xpos - halfW) / halfW * 2.0f - 1.0f;
            localY = (ypos - 0) / halfH * 2.0f - 1.0f;

            float step = 0.1f;
            float snappedX = round(localX / step) * step;
            float snappedY = round(localY / step) * step;       
            linePoints_left.push_back({snappedX, snappedY});
            lastview = View_left;
            snapX = snappedX;
            snapZ = snappedY;
            snapEvents.push_back({View_left, snapX, snapZ});
        }
        showSnapLines = true;
    }
}
void extractToAllverts(const Polyhedron &P) {
    
    for (auto &f : P.faces) {
        allvertices.push_back(P.verts[f[0]]);
        allvertices.push_back(P.verts[f[1]]);
        allvertices.push_back(P.verts[f[2]]);
    }
}

void init(){

    vector<Point> contour_xy = linePoints_top;
    vector<Point> contour_xz = linePoints_left;
    vector<Point> contour_yz = linePoints_front;

    auto dump_contour = [&](const vector<Point>& ctr, const char* name){
    cout << name << " (" << ctr.size() << " points):\n";
    for (size_t i = 0; i < ctr.size(); ++i)
        cout << "  ["<<i<<"] ("<<ctr[i].x<<", "<<ctr[i].y<<")\n";
    };
    auto dump_verts = [&](const vector<vec3>& ctr, const char* name){
        cout << name << " (" << ctr.size() << " points):\n";
        for (size_t i = 0; i < ctr.size(); ++i)
            cout << "  ["<<i<<"] ("<<ctr[i].x<<", "<<ctr[i].y<<", "<<ctr[i].z<<")\n";
    };
    
    auto planesXY = silhouettePlanes(contour_xy, ViewPlane::XY);
    auto planesXZ = silhouettePlanes(contour_xz, ViewPlane::XZ);
    auto planesYZ = silhouettePlanes(contour_yz, ViewPlane::YZ);
    dump_contour(contour_xy, "contour_xy");
    dump_plane(planesXY, "planesXY");
    dump_contour(contour_xz, "contour_xz");
    dump_plane(planesXZ, "planesXZ");
    dump_contour(contour_yz, "contour_yz");
    dump_plane(planesYZ, "planesYZ");
    
    
    Solid cylXY = buildCylinderXY(contour_xy, 1.0f);
    Solid cylXZ = buildCylinderXZ(contour_xz, 1.0f);
    Solid cylYZ = buildCylinderYZ(contour_yz, 1.0f);

    
    Polyhedron P = makePolyFromSolid(cylXY);          
    
    // P = P ∩ halfspaces_XZ
    // auto planesXZ = silhouettePlanes(contour_xz, ViewPlane::XZ); 
    for (auto &pl : planesXZ) {
        P = clipByPlane(P, pl);
    }
    cout<<"裁剪 XZ 後: verts="<<P.verts.size()
         <<", faces="<<P.faces.size()<<"\n";
    // //P = (P ∩ halfspaces_XZ) ∩ halfspaces_YZ
    // //auto planesYZ = silhouettePlanes(contour_yz, ViewPlane::YZ);
    for (auto &pl : planesYZ) {
        P = clipByPlane(P, pl);
    }
    cout<<"裁剪 YZ 後: verts="<<P.verts.size()
         <<", faces="<<P.faces.size()<<"\n";

    
    //dump_verts(P.verts, "verts");
    
    Polyhedron P1 = makePolyFromSolid(cylXZ);   
    for (auto &pl : planesXZ) {
        P1 = clipByPlane(P1, pl);
    }
    for (auto &pl : planesYZ) {
        P1 = clipByPlane(P1, pl);
    }
    
    Polyhedron P2 = makePolyFromSolid(cylYZ);  
    for (auto &pl : planesXZ) {
        P2 = clipByPlane(P2, pl);
    } 
    for (auto &pl : planesXY) {
        P2 = clipByPlane(P2, pl);
    }
    allvertices.clear();
    if (!P.verts.empty()) {
        extractToAllverts(P);
    }
    if (!P1.verts.empty()) {
        extractToAllverts(P1);
    }
    if (!P2.verts.empty()) {
        extractToAllverts(P2);
    } 

    
    glGenVertexArrays(1, &vao); //建立 VAO 的 ID 編號
    glGenBuffers(1, &vbo);  // 存放頂點的實體gpu記憶體區塊

    glBindVertexArray(vao); // 關於頂點的設定都存進這個 VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // 告訴 opengl接下來要對哪個 buffer 操作 類型是「頂點資料」
    glBufferData(GL_ARRAY_BUFFER, allvertices.size() * sizeof(vec3), allvertices.data(), GL_STATIC_DRAW);// 把資料丟進vbo
    
    glEnableVertexAttribArray(0); // 啟用頂點屬性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0); 

    glBindVertexArray(0); // 取消綁定 VAO
}

void initGL() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, allvertices.size()*sizeof(vec3), allvertices.data(), GL_DYNAMIC_DRAW);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, sizeof(vec3), (void*)0);
    glBindVertexArray(0);
}
void render3D(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/h, 0.1, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(3,3,3,  0,0,0,  0,0,1);

    // 更新 VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, allvertices.size()*sizeof(vec3), allvertices.data(), GL_DYNAMIC_DRAW);

    // 画线框
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(0,1,0);
    glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, (GLsizei)allvertices.size());
    glBindVertexArray(0);
}


// --------------------------------------------------
// 鍵盤按下處理
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action==GLFW_PRESS) std::cout<<"Key pressed: "<< key << "\n";
    //std::cout << "Key pressed: " << key << ", action: " << action << std::endl;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_C) {  // Clear
            linePoints_top.clear();
            linePoints_front.clear();
            linePoints_left.clear();
            shouldFill = false;
        }
        if (key == GLFW_KEY_F) {  // Fill
            shouldFill = true;
            init();
        }
    }
    // 用鍵盤（上下左右）控制右上視窗旋轉
    const float angleStep = M_PI / 90.0f;
    if (action == GLFW_PRESS || action == GLFW_REPEAT){
        
        switch (key){
            case GLFW_KEY_LEFT:
                theta += angleStep;
                break;
            case GLFW_KEY_RIGHT:
                theta -= angleStep;
                break;
            case GLFW_KEY_UP:
                phi += angleStep;
                break;
            case GLFW_KEY_DOWN:
                phi -= angleStep;
                break;
            case GLFW_KEY_MINUS:
                theta = 0.0f;
                phi = 0.0f;
                transformMat = mat4x4(1);
            return;
        }
        transformMat = rotate(mat4x4(1),
                                   phi, vec3(1.0f, 0.0f, 0.0f));
        transformMat = rotate(transformMat,
                                   theta, vec3(0.0f, 0.0f, 1.0f));
    }
}
// --------------------------------------------------
//      處理四個畫布
// --------------------------------------------------

void render(){
    if (allvertices.empty()) return;
    glBindVertexArray(vao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // allVertices 裡頭已經是三角形清單了，一次 draw 出來
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)allvertices.size());
    
    //  // 先畫 XY
    // glDepthRange(0.0, 0.33);
    //  glColor3f(0, 0, 1);
    //  glDrawArrays(GL_TRIANGLES, 0, vertsXY);
    // // 再畫 XZ
    // glDepthRange(0.33, 0.66);
    // glColor3f(0, 1, 0);
    // glDrawArrays(GL_TRIANGLES, vertsXY, vertsXZ);
    //  // 最後畫 YZ
    //  glDepthRange(0.66, 1.0);
    //  glColor3f(1, 0, 0);
    //  glDrawArrays(GL_TRIANGLES, vertsXY + vertsXZ, vertsYZ);
    //  glDepthRange(0.0, 1.0);

    glBindVertexArray(0);
    glBindVertexArray(0);

}
static const float RANGE = 10.f;
void setupViewport(int x, int y, int w, int h){
    glViewport(x, y, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void setupViewport_render(int x, int y, int w, int h){
    glViewport(x, y, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (w/2.0) / (h/2.0), 0.1, 100.0);
    gluLookAt(
        cameraPos.x, cameraPos.y, cameraPos.z,  // 相機 
        0.0, 0.0, 0.0,  // 看向原點
        cameraUp.x, cameraUp.y, cameraUp.z   // 上方向
    );
    glMultMatrixf(&transformMat[0][0]);

}

void draw_left_canvas(const vector<Point>& points) {
    glLineWidth(3.0f);
    glColor3f(0, 0, 1); // 藍色
    glBegin(GL_LINE_STRIP);
    for (auto& p : points)
        glVertex2f(p.x, p.y);
    glEnd();
}

// --------------------------------------------------
//      畫線
// --------------------------------------------------
void draw_grid(int size = 10){
    glBegin(GL_LINES);
    glLineWidth(1.0f);
    glColor3f(0.3, 0.3, 0.3);
    for(int i = 1; i< size; i++){
        // 垂直線
        glVertex2f(i, -size);
        glVertex2f(i, size);
        glVertex2f(-i, -size);
        glVertex2f(-i, size);
        // 水平線
        glVertex2f(-size, i);
        glVertex2f(size, i);
        glVertex2f(-size, -i);
        glVertex2f(size, -i);
    }
    glEnd();

    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex2f(0, 0);
        glVertex2f(size, 0);
        glColor3f(0.4, 0, 0);
		glVertex2f(0, 0);
		glVertex2f(-size, 0);

        glColor3f(0, 1, 0);
        glVertex2f(0, 0);
        glVertex2f(0, size);
        glColor3f(0, 0.4, 0);
		glVertex2f(0, 0);
		glVertex2f(0, -size);
    glEnd();
}

// 畫線輔助線
void drawSnapLinesFront() {
    for (auto &e : snapEvents) {
        if (e.view == View_front) 
          continue;  // 跳過自己在 YZ 視圖裡的事件
    
        glColor3f(0.5,0.5,0.5);
        glLineWidth(2);
        glBegin(GL_LINES);
    
          // 垂直線：Z = snapZ
          glVertex2f(e.a, -1.0f);
          glVertex2f(e.a, +1.0f);
    
          // 水平線： Y = snapY
          glVertex2f(-1.0f, e.b);
          glVertex2f(+1.0f, e.b);
    
        glEnd();
      }
}

void drawSnapLinesTop() {
    for(auto &e : snapEvents){
        if(e.view == View_top)
            continue;
        glColor3f(0.5,0.5,0.5);
        glLineWidth(2);
        glBegin(GL_LINES);
        // 垂直線：X = snapX
        glVertex2f(e.a, -1.f);
        glVertex2f(e.a, +1.f);
        // 水平線：Y = snapY
        glVertex2f(-1.f, e.b);
        glVertex2f(+1.f, e.b);
        glEnd();
}
}

void drawSnapLinesLeft() {
    for (auto &e : snapEvents){
        if(e.view == View_left)
            continue;
        glColor3f(0.5,0.5,0.5);
        glLineWidth(2);
        glBegin(GL_LINES);
        // 垂直線：X = snapX
        glVertex2f(e.a, -1.f);
        glVertex2f(e.a, +1.f);
        // 水平線：Z = snapZ
        glVertex2f(-1.f, e.b);
        glVertex2f(+1.f, e.b);
        glEnd();
}
}
//畫布輔助線
void draw_guidelines() {
    glLineWidth(3.0f);
    // 畫 X=0 軸
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
      glVertex2f(-1.0f, 0.0f);
      glVertex2f( 1.0f, 0.0f);
    glEnd();
    // 畫 Y=0 軸
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
      glVertex2f(0.0f, -1.0f);
      glVertex2f(0.0f,  1.0f);
    glEnd();
    glLineWidth(1.0f);
}



int main() {
    glfwInit();
    
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // #ifdef __APPLE__
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // #endif
    GLFWwindow* window = glfwCreateWindow(800, 600, "畫布", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// 穩定幀數
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    
    // 第一次测试
    

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // // 1) 新一幀 ImGui
        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();

        // 2) ImGui 繪製
        // ImGui::Begin("CSG 操作");
        // if(ImGui::Button("交集(Intersect)")) {
        //     currentOP = CSG_op::INTERSECT;
        // }
        // if(ImGui::Button("聯集(Union)")) {
        //     currentOP = CSG_op::UNION;
        // }
        // if(ImGui::Button("差集(Difference)")) {
        //     currentOP = CSG_op::DIFFERENCE;
        // }
        // ImGui::Text("目前：%s",
        //     currentOP == CSG_op::INTERSECT ? "交集" :
        //     currentOP == CSG_op::UNION ? "聯集" :
        //     currentOP == CSG_op::DIFFERENCE ? "差集" : "未知");
        // ImGui::End();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //glClear(GL_COLOR_BUFFER_BIT);
        

        // 左下邊畫線視窗（yz）
        setupViewport(0, 0, width / 2, height/2);
        draw_grid();
        draw_guidelines();
        draw_left_canvas(linePoints_front);
        drawSnapLinesFront();
        glFlush(); // 送到 GPU 去執行

        // 左上邊畫線視窗（xy）
        setupViewport(0, height/2, width / 2, height/2);
        draw_grid();
        draw_guidelines();
        draw_left_canvas(linePoints_top);
        drawSnapLinesTop();
        glFlush();

        // 右下邊畫線視窗（xz）
        setupViewport(width / 2, 0, width / 2, height/2);
        draw_grid();
        draw_guidelines();
        draw_left_canvas(linePoints_left);
        drawSnapLinesLeft();
        glFlush();

        // 右上邊生成視窗
        setupViewport_render(width / 2, height/2, width / 2, height/2);
        
        draw_grid();
        render();
        glFlush();
        //draw_right_shape();
        
        //分隔線
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, 0, height, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor3f(0, 0, 0);
        glBegin(GL_LINES);
        glVertex2f(width / 2, 0);
        glVertex2f(width / 2, height);
        glVertex2f(0, height / 2);
        glVertex2f(width, height / 2);
        glEnd();
    
        //主畫面渲染
        //renderImGuiOver();
        glfwSwapBuffers(window);
    }
    std::cout<<"-> Window created, entering loop\n";
    
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    glfwTerminate();
}
