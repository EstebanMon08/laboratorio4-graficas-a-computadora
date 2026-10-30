#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Vec3 { float x, y, z; };
struct Face { int a, b, c; };

static const int WIDTH = 800;
static const int HEIGHT = 800;


bool loadOBJ(const std::string& path, std::vector<Vec3>& verts, std::vector<Face>& faces) {
    std::ifstream file(path);
    if (!file) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            verts.push_back(v);
        } else if (tag == "f") {
            std::vector<int> idx;
            std::string token;
            while (ss >> token) {
                int i = std::stoi(token);
                if (i < 0) i = (int)verts.size()
                idx.push_back(i - 1);
            }
            for (size_t k = 1; k + 1 < idx.size(); k++)
                faces.push_back({ idx[0], idx[k], idx[k + 1] });
        }
    }
    return true;
}


void normalize(std::vector<Vec3>& verts) {
    Vec3 mn = verts[0], mx = verts[0];
    for (auto& v : verts) {
        mn.x = std::min(mn.x, v.x); mx.x = std::max(mx.x, v.x);
        mn.y = std::min(mn.y, v.y); mx.y = std::max(mx.y, v.y);
        mn.z = std::min(mn.z, v.z); mx.z = std::max(mx.z, v.z);
    }
    Vec3 c = { (mn.x + mx.x) / 2, (mn.y + mx.y) / 2, (mn.z + mx.z) / 2 };
    float size = std::max({ mx.x - mn.x, mx.y - mn.y, mx.z - mn.z });
    for (auto& v : verts) {
        v.x = (v.x - c.x) / size;
        v.y = (v.y - c.y) / size;
        v.z = (v.z - c.z) / size;
    }
}


Vec3 rotate(Vec3 p, float ax, float ay) {
    // Eje X
    float y1 = p.y * cosf(ax) - p.z * sinf(ax);
    float z1 = p.y * sinf(ax) + p.z * cosf(ax);
    // Eje Y
    float x2 = p.x * cosf(ay) + z1 * sinf(ay);
    float z2 = -p.x * sinf(ay) + z1 * cosf(ay);
    return { x2, y1, z2 };
}


Vector2 toScreen(Vec3 p, float scale) {
    return { WIDTH / 2.0f + p.x * scale, HEIGHT / 2.0f - p.y * scale };
}

int main(int argc, char** argv) {
    std::string path = (argc > 1) ? argv[1] : "suzanne.obj";

    std::vector<Vec3> verts;
    std::vector<Face> faces;
    if (!loadOBJ(path, verts, faces) || verts.empty()) {
        std::cerr << "No se pudo cargar el modelo: " << path << "\n";
        return 1;
    }
    normalize(verts);
    std::cout << "Vertices: " << verts.size() << "  Triangulos: " << faces.size() << "\n";

    InitWindow(WIDTH, HEIGHT, "Wireframe OBJ");
    SetTargetFPS(60);

    float angleX = 0.0f, angleY = 0.0f;
    const float scale = WIDTH * 0.8f;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT))  angleY -= 0.03f;
        if (IsKeyDown(KEY_RIGHT)) angleY += 0.03f;
        if (IsKeyDown(KEY_UP))    angleX -= 0.03f;
        if (IsKeyDown(KEY_DOWN))  angleX += 0.03f;
        if (IsKeyPressed(KEY_R))  { angleX = 0; angleY = 0; }


        std::vector<Vector2> projected(verts.size());
        for (size_t i = 0; i < verts.size(); i++)
            projected[i] = toScreen(rotate(verts[i], angleX, angleY), scale);

        BeginDrawing();
        ClearBackground(BLACK);


        for (const Face& f : faces) {
            DrawLineV(projected[f.a], projected[f.b], WHITE);
            DrawLineV(projected[f.b], projected[f.c], WHITE);
            DrawLineV(projected[f.c], projected[f.a], WHITE);
        }

        EndDrawing();

        if (IsKeyPressed(KEY_S)) TakeScreenshot("screenshot.png");
    }

    CloseWindow();
    return 0;
}