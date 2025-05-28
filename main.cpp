#include<iostream>
#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h> 
#include <OpenGL/glu.h>
#include <vector>
#include <glm/glm.hpp>
#include "cone_builder.hpp"

GLuint vao, vbo;
using namespace glm;
using namespace std;



vector<Point> linePoints_front;
vector<Point> linePoints_left;
vector<Point> linePoints_top;
vector<vec3> Triangle_vertices_front;
vector<vec3> Triangle_vertices_top;
vector<vec3> Triangle_vertices_left;



bool shouldFill = false;

// 滑鼠按下記錄點
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        ypos = height - ypos;

        

        int halfW = width / 2;
        int halfH = height / 2;

        float nx, ny;

        if (xpos < halfW && ypos < halfH) {
            // 左下畫布 (YZ)
            nx = xpos / halfW * 2.0f - 1.0f;
            ny = ypos / halfH * 2.0f - 1.0f;
            linePoints_front.push_back({nx, ny});
        } 
        else if (xpos < halfW && ypos >= halfH) {
            // 左上畫布 (XY)
            nx = xpos / halfW * 2.0f - 1.0f;
            ny = (ypos - halfH) / halfH * 2.0f - 1.0f;
            linePoints_top.push_back({nx, ny});
        } 
        else if (xpos >= halfW && ypos < halfH) {
            // 右下畫布 (XZ)
            nx = (xpos - halfW) / halfW * 2.0f - 1.0f;
            ny = (ypos - 0) / halfH * 2.0f - 1.0f;
            linePoints_left.push_back({nx, ny});
        }
    }
}
// 用滑鼠旋轉座標
float lastX = 400, lastY = 300;
bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f;
float radius = 3.0f;
vec3 cameraPos(0.0f, 0.0f, radius);
vec3 cameraFront(0.0f, 0.0f, -1.0f);
vec3 cameraUp(0.0f, 1.0f, 0.0f);

//確認滑鼠座標位置
bool isInUpperRight(int x, int y, int windowWidth, int windowHeight) {
    return (x > windowWidth / 2 && y < windowHeight / 2);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    int winW, winH;
    glfwGetFramebufferSize(window, &winW, &winH);

    if (!isInUpperRight(xpos, ypos, winW, winH))
        return;// 不在右上角，不處理滑鼠旋轉
    
    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;// 計算偏移量
    float yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;// 控制滑鼠靈敏度
    yoffset *= sensitivity;

    yaw += xoffset;// 控制左右旋轉
    pitch += yoffset;// 控制上下旋轉

    // 限制旋轉角度
    // note: yaw 不需要限制，因為它可以繞水平軸無限旋轉
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    
    float radYaw = radians(yaw);// 把角度換成弧度
    float radPitch = radians(pitch);

    cameraPos.x = radius * cos(radPitch) * cos(radYaw);// 計算鏡頭位置
    cameraPos.y = radius * sin(radPitch);
    cameraPos.z = radius * cos(radPitch) * sin(radYaw);

}

void init(){
    vector<Point> contour_xy = linePoints_top;
    Triangle_vertices_top = buildCone_xy(contour_xy, 0.5f);

    vector<Point> contour_xz = linePoints_left;
    Triangle_vertices_left = buildCone_xz(contour_xz, 0.5f);

    vector<Point> contour_yz = linePoints_front;
    Triangle_vertices_front = buildCone_yz(contour_yz, 0.5f);

    glGenVertexArrays(1, &vao); //建立 VAO 的 ID 編號
    glGenBuffers(1, &vbo);  // 存放頂點的實體gpu記憶體區塊

    glBindVertexArray(vao); // 關於頂點的設定都存進這個 VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // 告訴 opengl接下來要對哪個 buffer 操作 類型是「頂點資料」

    glBufferData(GL_ARRAY_BUFFER, Triangle_vertices_top.size() * sizeof(vec3), Triangle_vertices_top.data(), GL_STATIC_DRAW);// 把資料丟進vbo
    glBufferData(GL_ARRAY_BUFFER, Triangle_vertices_left.size() * sizeof(vec3), Triangle_vertices_left.data(), GL_STATIC_DRAW); 
    glBufferData(GL_ARRAY_BUFFER, Triangle_vertices_front.size() * sizeof(vec3), Triangle_vertices_front.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // 啟用頂點屬性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0); 

    glBindVertexArray(0); // 取消綁定 VAO
}

// 鍵盤按下處理
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
}

void render(){
    if (Triangle_vertices_top.empty()|| Triangle_vertices_left.empty()) return;
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, Triangle_vertices_top.size());
    glDrawArrays(GL_TRIANGLES, 0, Triangle_vertices_left.size());
    glDrawArrays(GL_TRIANGLES, 0, Triangle_vertices_front.size());
    glBindVertexArray(0);
}
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

}

void draw_left_canvas(const vector<Point>& points) {
    glLineWidth(2.0f);
    glColor3f(0, 0, 1); // 藍色
    glBegin(GL_LINE_STRIP);
    for (auto& p : points)
        glVertex2f(p.x, p.y);
    glEnd();
}

void draw_right_shape(const vector<Point>& points) {
    glLineWidth(2.0f);
    glColor3f(1, 0, 0); // 紅色
    if (shouldFill && points.size() >= 3) {
        glBegin(GL_POLYGON);
        for (auto& p : points)
            glVertex2f(p.x, p.y);
        glEnd();
    }
}

void draw_grid(int size = 10){
    glBegin(GL_LINES);
    glColor3f(0.3, 0.3, 0.3);
    for(int i = 1; i< size; i++){
        glVertex2f(i, -size);
        glVertex2f(i, size);
        glVertex2f(-i, -size);
        glVertex2f(-i, size);
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

int main() {
    glfwInit();
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "畫布", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);// 穩定幀數
    glEnable(GL_DEPTH_TEST);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetKeyCallback(window, key_callback);
    

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BU