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
Vec3f light_dir(0, 0, 1);

//计算重心坐标
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {  
    Vec3f s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = s[0] ^ s[1];
    if (std::abs(u[2]) > 1e-2) 
        return Vec3f(1 - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x + 1) * width / 2. + 0.5), int((v.y + 1) * width / 2. + 0.5), v.z);
}

//Bresenham算法
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

    int dx = p1.x - p0.x;
    int dy = std::abs(p1.y - p0.y);

    int error = dx / 2;
    int ystep = (p0.y < p1.y) ? 1 : -1;
    int y = p0.y;


    for (int x = p0.x; x <= p1.x; ++x) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }

    error -= dy;
    if (error < 0) {
        y += ystep;
        error += dx;
    }
}

void triangle(Vec3f* pts, Vec2f* tex, float* zbuffer, TGAImage& image, TGAImage& texture, float intensity) {
    Vec2f bboxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2f bboxmax(0, 0);
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    //确定包围盒的范围
    for (int i = 0; i < 3; i++) {
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));
        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    Vec3f P;
    
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc = barycentric(pts[0], pts[1], pts[2], P);
            //std::cout << bc << std::endl;
            if (bc.x < 0 || bc.y < 0 || bc.z < 0)//P不在三角形中
                continue;
            P.z = 0;
            Vec2f tex_coords(0, 0);
            for (int i = 0; i < 3; i++) {
                P.z += pts[i][2] * bc[i];
                tex_coords.x += tex[i][0] * bc[i]; //插值纹理坐标
                tex_coords.y += tex[i][1] * bc[i];
            }
               
            if (zbuffer[int(P.x + P.y * width)] < P.z) {//摄像机看向z轴负半轴？
                zbuffer[int(P.x + P.y * width)] = P.z;
                //利用纹理坐标获取对应的颜色
                TGAColor color = texture.get(tex_coords.x * texture.get_width(), tex_coords.y * texture.get_height());
                for (int i = 0; i < 3; ++i) {
                    color[i] *= intensity;
                }

                image.set(P.x, P.y, color);
            }               
        }
    }
}

int main(int argc, char** argv) {
    TGAImage image(width, height, TGAImage::RGB);

    TGAImage texture;
    if (!texture.read_tga_file("obj/african_head_diffuse.tga")) { //读取纹理贴图
        std::cerr << "failed to load texture file!" << std::endl;
        return -1;
    }
    texture.flip_vertically();

    float* zbuffer = new float[width * height];
    for (int i = 0; i < width * height; i++)
        zbuffer[i] = -std::numeric_limits<float>::max();
    
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> tex_face = model->texface(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec2f tex_coords[3];

        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = world2screen(v);
            //std::cout << screen_coords[j] << std::endl;
            world_coords[j] = v;
            //std::cout << world_coord[j] << std::endl;
            tex_coords[j] = model->texcoord(tex_face[j]);
        }
        Vec3f n = (world_coords[1] - world_coords[0]) ^ (world_coords[2] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;//dot product
        if (intensity > 0)
            triangle(screen_coords, tex_coords, zbuffer, image, texture, intensity);
    }

    /*for (int i = 0; i < width*height; i++)
        std::cout << zbuffer[i] << std::endl;*/

    image.flip_vertically(); 
    image.write_tga_file("output.tga");
    delete model;
    
    return 0;
}

