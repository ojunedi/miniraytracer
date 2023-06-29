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
    std::vector<vec3> framebuffer(width * height);
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            framebuffer[i+j*width] = vec3(j/float(height),i/float(width), 0);
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

    vec3 center;
    double radius;

    // constructor
    Sphere(const vec3 &c, const double &r) : center(c), radius(r) {}

    double discriminant(double a, double b, double c) const {
        return (b * b - (4 * a * c));
    }

    bool ray_intersect(const vec3 &p, const vec3 &dir, double &t0) const {

        vec3 L = center - p;
        float tca = L*dir;
        float d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
        // Vec3f L = center - p;
        // double a = dir * dir;
        // double b = 2 * (dir * L);
        // double c = (L * L) - (radius * radius);
        // if (discriminant(a, b, c) < 0) {
        //     return false;
        // } 
        // return true;
    }

};

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const Sphere &sphere) {
    double sphere_dist = std::numeric_limits<float>::max();
    if (!sphere.ray_intersect(orig, dir, sphere_dist)) {
        return vec3(0.2, 0.7, 0.8);
    }
    return vec3(0.4, 0.4, 0.3);
}   






int main() {
    render();
    return 0;
}