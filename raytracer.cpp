#include <limits>
#include <utility>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"


struct Material {
    Material(const vec3 &color) : diffuse_color(color) {}
    Material() : diffuse_color() {}
    vec3 diffuse_color;
};

struct Sphere {

    vec3 center;
    double radius;
    Material material;

    // constructor
    Sphere(const vec3 &c, const double &r, const Material &m) : center(c), radius(r), material(m) {}

    double discriminant(double a, double b, double c) const {
        return (b * b - (4 * a * c));
    }

    bool scene_intersect(const vec3 &orig, const vec3 &dir, const std::vector<Sphere> &spheres, vec3 &hit, vec3 &N, Material &material) {
        double spheres_dist = std::numeric_limits<double>::max();
        for (size_t i=0; i < spheres.size(); i++) {
            double dist_i;
            if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
                spheres_dist = dist_i;
                hit = orig + dir*dist_i;
                N = (hit - spheres[i].center).normalize();
                material = spheres[i].material;
            }
        }
        return spheres_dist < 1000;
}

    bool ray_intersect(const vec3 &p, const vec3 &dir, double &t0) const {

        vec3 L = center - p;
        double tca = L*dir;
        double d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        double thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        double t1 = tca + thc;
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

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const Sphere &sphere, int j, int i, std::vector<std::pair<int,int>> &intersections) {
    double sphere_dist = std::numeric_limits<double>::max();

    if (!sphere.ray_intersect(orig, dir, sphere_dist)) {
        return vec3(0.2, 0.7, 0.8);
    }
    intersections.push_back({j, i});
    return vec3(0.4, 0.4, 0.3);
}   


void render(const Sphere &sphere) {

    const int width = 1024;
    const int height = 768;
    const double fov = atan(1)*(4/double(3)); // pi/3

    // contains data representing all the pixels in a frame
    std::vector<vec3> framebuffer(width * height);
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            framebuffer[i+j*width] = vec3(j/double(height),i/double(width), 0);
        }        
    }

    std::vector<std::pair<int, int>> intersections;

    // using trig to determine x and y coordinates 
    #pragma omp parallel for
    for (size_t j = 0; j<height; j++) {
        for (size_t i = 0; i<width; i++) {
            double x =  (2*(i + 0.5)/(double)width  - 1)*tan(fov/2.)*width/(double)height;
            double y = -(2*(j + 0.5)/(double)height - 1)*tan(fov/2.);
            vec3 dir = vec3(x, y, -1).normalize();
            framebuffer[i+j*width] = cast_ray(vec3(0,0,0), dir, sphere, j, i, intersections);
        }
    }

    // debugging purposes
    for (int x = 0; x < 10; ++x) {
        std::cout << intersections[x].first << " " << intersections[x].second << std::endl;
    }



    std::ofstream ofs; // save the framebuffer to file
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height*width; ++i) {
        for (size_t j = 0; j<3; j++) {
            ofs << (char)(int)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();

}







int main() {
    Material white(vec3(255,255,255));
    Sphere sphere(vec3(-3, 0, -16), 2, white);
    render(sphere);
    return 0;
}