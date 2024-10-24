#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), tex_coords_(), tex_faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vt ")) {//读取纹理坐标
            iss >> trash >> trash;
            Vec2f vt;
            for (int i = 0; i < 2; ++i) {
                iss >> vt.raw[i];
            }
            tex_coords_.push_back(vt);
        }
        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;//用于保存顶点index
            std::vector<int> tf;//保存纹理index
            int itrash, idx, t_idx;
            iss >> trash;
            while (iss >> idx >> trash >> t_idx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                t_idx--;
                tf.push_back(t_idx);
            }
            faces_.push_back(f);
            tex_faces_.push_back(tf);//保存每个面的纹理坐标索引
        }
    }
    std::cerr << " v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
    std::cerr << " vt# " << tex_coords_.size() << " tex_faces_# " << tex_faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

//获取纹理坐标
Vec2f Model::texcoord(int i) {
    return tex_coords_[i];
}

//获取面顶点的纹理坐标索引
std::vector<int> Model::texface(int idx) {
    return tex_faces_[idx];
}

