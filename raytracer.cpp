#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"


void render() {

    const int width = 1024;
    const int height = 768;

    // contains data representing all the pixels in a frame
    std::vector<Vec3f> framebuffer(width * height);
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            framebuffer[i+j*width] = Vec3f(j/float(height),i/float(width), 0);
        }        
    }

    std::ofstream ofs; // save the framebuffer to file
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height*width; ++i) {
        for (size_t j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();

}

struct Sphere {

    Vec3f center;
    double radius;

    // constructor
    Sphere(const Vec3f &c, const double &r) : center(c), radius(r) {}

    double discriminant(double a, double b, double c) const {
        return (b * b - (4 * a * c));
    }

    bool ray_intersect(const Vec3f &p, const Vec3f &dir, double &t0) const {
        Vec3f L = center - p;
        double a = dir * dir;
        double b = 2 * (dir * L);
        double c = (L * L) - (radius * radius);
        if (discriminant(a, b, c) < 0) {
            return false;
        } 
        return true;
    }

};




int main() {
    render();
    return 0;
}