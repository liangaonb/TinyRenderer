#include <iostream>
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);

const int width = 800;
const int height = 800;

Model* model = new Model("obj/african_head.obj");
Vec3f light_dir(0, 0, -1);

Vec3f barycentric(Vec2i* pts, Vec2i p) {
    Vec3f u = Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - p[0]) ^
                Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - p[1]);//重心坐标的计算有公式
    if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    return Vec3f(1 - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }

    for (int x = p0.x; x <= p1.x; x++) {
        float t = (float)(x - p0.x) / (p1.x - p0.x);
        int y = p0.y * (1 - t) + p1.y * t;
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}

void triangle(Vec2i* pts, TGAImage& image, TGAColor color) {
    Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i bboxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
    //确定包围盒的范围
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec2i p;
    for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++) {
        for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++) {
            Vec3f bc = barycentric(pts, p);
            if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;
            image.set(p.x, p.y, color);
        }
    }
}

int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coord[3];
        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
            world_coord[j] = v;
        }
        Vec3f n = (world_coord[2] - world_coord[0]) ^ (world_coord[1] - world_coord[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0)
            triangle(screen_coords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
    }
    //std::cout << pts[2][0] << std::endl;
    image.flip_vertically(); 
    image.write_tga_file("output.tga");
    //so ignore this
    return 0;
}

