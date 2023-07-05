#include <limits>
#include <utility>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"


struct Material {
    Material(const vec3 &color, const vec2 &a, const float &sc) : diffuse_color(color), albedo(a), shiny_constant(sc) {}
    Material() :  diffuse_color(), albedo(1,0), shiny_constant() {}
    vec3 diffuse_color;
    vec2 albedo;
    float shiny_constant;
};

struct Light {
    Light(const vec3 &p, const float &i) : position(p), intensity(i) {}
    vec3 position;
    float intensity;
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


    bool ray_intersect(const vec3 &p, const vec3 &dir, double &t0) const {

        // GEOMETRIC
        vec3 L = center - p;
        double tca = L*dir;
        double d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        double thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        double t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        return t0 >= 0;
        // ANALYTIC
        // vec3 L = center - p;
        // double a = dir * dir;
        // double b = 2 * (dir * L);
        // double c = (L * L) - (radius * radius);
        // return discriminant(a, b, c) >= 0;
    }

};

vec3 reflect(const vec3 &I, const vec3 &N) {
    return I - N*2.f*(I*N);
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

vec3 cast_ray(const vec3 &orig, const vec3 &dir, int j, int i, std::vector<std::pair<int,int>> &intersections, 
                const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {

    vec3 point, N;
    Material material;

    if (!scene_intersect(orig, dir, spheres, point, N, material)) {
        // return vec3(0.2, 0.7, 0.8); // blueish color
        return vec3(0.5,0.5,0.5); // grey
    }
    intersections.push_back({j, i});

    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        vec3 light_dir = (lights[i].position - point).normalize();
        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir*N);
        specular_light_intensity += pow(std::max(0.f ,reflect(light_dir, N) * dir), material.shiny_constant) * lights[i].intensity;
    }
    return (material.diffuse_color * diffuse_light_intensity * material.albedo[0]) 
            + (vec3(1.0, 1.0, 1.0) * specular_light_intensity * material.albedo[1]);

}   


void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {

    const int width = 1024;
    const int height = 768;
    const double fov = atan(1)*(4/double(2)); // pi/2

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
            framebuffer[i+j*width] = cast_ray(vec3(0,0,0), dir, j, i, intersections, spheres, lights);
        }
    }

    // debugging purposes
    for (int x = 0; x < 10; ++x) {
        std::cout << intersections[x].first << " " << intersections[x].second << std::endl;
    }

    std::ofstream fout; // save the framebuffer to file
    fout.open("./out.ppm");
    fout << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height*width; ++i) {
        for (size_t j = 0; j<3; j++) {
            fout << (char)(int)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    fout.close();

}

int main() {

    Material ivory( vec3(0.4, 0.4, 0.3), vec2(0.6, 0.3),50.);
    Material red_rubber(vec3(0.3, 0.1, 0.1), vec2(0.9, 0.1), 10.);
    Material turquiose(vec3(0, 1, 1), vec2(0.3, 0.5), 25.);

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(vec3(-3,    10,   -16), 2,      ivory));
    spheres.push_back(Sphere(vec3(-1.0, -1.5, -12), 2, turquiose));
    spheres.push_back(Sphere(vec3( 1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(vec3( 7,    5,   -18), 4,      ivory));
    spheres.push_back(Sphere(vec3(-5, 1, -10), 2, turquiose));
    spheres.push_back(Sphere(vec3(-7, -5, -10), 1, red_rubber));
    spheres.push_back(Sphere(vec3(-15,    -10,   -16), 2,      ivory));
    
    std::vector<Light> lights;
    lights.push_back(Light(vec3(-20,20,20),1.5));
    lights.push_back(Light(vec3( 30, 50, -25), 1.8));
    lights.push_back(Light(vec3( 30, 20,  30), 1.7));
    lights.push_back(Light(vec3(20,20,20), 0.75));
    render(spheres, lights);

    return 0;
}