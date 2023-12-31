#include <limits>
#include <utility>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"


struct Material {
    Material(const double &r, const vec4 &a, const vec3 &color, const double &sc) : refractive_index(r), albedo(a), diffuse_color(color), shiny_constant(sc) {}
    Material() :  refractive_index(1), albedo(1,0,0,0), diffuse_color(), shiny_constant() {}
    double refractive_index;
    vec4 albedo;
    vec3 diffuse_color;
    double shiny_constant;
};

struct Light {
    Light(const vec3 &p, const double &i) : position(p), intensity(i) {}
    vec3 position;
    double intensity;
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

vec3 refract(const vec3 &I, const vec3 &N, const double &refractive_index) {
    
    // Snell's Law
    double cosi = - std::max(-1.f, std::min(1.f, I*N));
    double etai = 1, etat = refractive_index;
    vec3 n = N;
    if (cosi < 0) {
        cosi = -cosi;
        std::swap(etai, etat);
        n = -N;
    }
    double eta = etai / etat;
    double k = 1 - eta*eta*(1-cosi*cosi);
    return k < 0 ? vec3(0,0,0): I * eta + n*(eta * cosi - sqrt(k));

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

    double checkerboard_dist = std::numeric_limits<double>::max();
    double d = -(orig.y+4)/dir.y;
    vec3 point = orig + dir*d;
    if (fabs(dir.y) > 1e-3){
        if (d > 0 and fabs(point.x) < 10 and point.z < -10 and point.z > -30 and d < spheres_dist) {
            checkerboard_dist = d;
            hit = point;
            N = vec3(0,1,0);
            material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? vec3(1,1,1) : vec3(1, .7, .3);
            material.diffuse_color = material.diffuse_color * 0.3;
        }
    }

    return std::min(spheres_dist, checkerboard_dist) < 1000;
}

vec3 cast_ray(const vec3 &orig, const vec3 &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth=0) {

    vec3 point, N;
    Material material;
    double epsilon = 1e-3;


    if (depth > 4 || !scene_intersect(orig, dir, spheres, point, N, material)) {
        return vec3(0.2, 0.7, 0.8); // blueish color
        // return vec3(0.5,0.5,0.5); // grey
        // return vec3(0.1,0.1,0.1);
    }

    vec3 reflect_dir = reflect(dir, N).normalize();
    vec3 refract_dir = refract(dir, N, material.refractive_index).normalize();
    vec3 reflect_origin;
    vec3 refract_origin;
    if (reflect_dir * N < 0) {
        reflect_origin = point - N * epsilon;
    } else {
        reflect_origin = point + N * epsilon;
    }
    if (refract_dir * N < 0) {
        refract_origin = point - N * epsilon;
    } else {
        refract_origin = point + N * epsilon;
    }
    std::vector<std::pair<int,int>> dummy = {{1,1}};
    vec3 reflect_color = cast_ray(reflect_origin, reflect_dir, spheres, lights, depth + 1);
    vec3 refract_color = cast_ray(refract_origin, reflect_dir, spheres, lights, depth + 1);
    double diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        vec3 light_dir = (lights[i].position - point).normalize();
        // shadows 
        double light_distance = (lights[i].position - point).norm();
        vec3 shadow_origin;
        // perturbing point in direction of normal to combat self shadowing
        // https://web.cse.ohio-state.edu/~shen.94/681/Site/Slides_files/shadow.pdf
        // slides 14-17
        // in my case without pertubation, the shadows were speckled all over the spheres
        if (light_dir * N < 0) {
            shadow_origin = point - N*epsilon;
        } else {
            shadow_origin = point + N*epsilon;
        }
        vec3 shadow_point, shadow_normal;
        Material temp_material;
        if (scene_intersect(shadow_origin, light_dir, spheres, shadow_point, shadow_normal, temp_material) and (shadow_point - shadow_origin).norm() < light_distance) {
            continue;
        }
        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir*N);
        specular_light_intensity += pow(std::max(0.f ,reflect(light_dir, N) * dir), material.shiny_constant) * lights[i].intensity;
    }
    return ((material.diffuse_color * diffuse_light_intensity * material.albedo[0]) 
            + (vec3(1.0, 1.0, 1.0) * specular_light_intensity * material.albedo[1]) 
            + (reflect_color*material.albedo[2])
            + (refract_color*material.albedo[3]));

}   


void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {

    const int width = 1024;
    const int height = 768;
    const double fov = (atan(1)*4) / 2.; // pi/2

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
            framebuffer[i+j*width] = cast_ray(vec3(0,0,0), dir, spheres, lights);
        }
    }

    // debugging purposes
    // for (int x = 0; x < std::min(10, (int)intersections.size()); ++x) {
    //     std::cout << intersections[x].first << " " << intersections[x].second << std::endl;
    // }

    std::ofstream fout; // save the framebuffer to file
    fout.open("./out.ppm");
    fout << "P6\n" << width << " " << height << "\n255\n";
    // for (size_t i = 0; i < height*width; ++i) {
    //     for (size_t j = 0; j<3; j++) {
    //         fout << (char)(int)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    //     }
    // }

    for (size_t i = 0; i < height * width; ++i) {
        vec3 &c = framebuffer[i];
        double max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1) {
            c = c * (1./max);
        }
        for (size_t j = 0; j < 3; j++) {
            fout << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }

    fout.close();

}

int main() {

    Material  turquiose(1.0, vec4(0.3, 0.5, 0.2, 0.0),  vec3(0.0, 1.0, 1.0), 25.);
    Material      ivory(1.0, vec4(0.6,  0.3, 0.1, 0.0), vec3(0.4, 0.4, 0.3),   50.);
    Material      glass(1.5, vec4(0.0,  0.5, 0.1, 0.8), vec3(0.6, 0.7, 0.8),  125.);
    Material red_rubber(1.0, vec4(0.9,  0.1, 0.0, 0.0), vec3(0.3, 0.1, 0.1),   10.);
    Material     mirror(1.0, vec4(0.0, 10.0, 0.8, 0.0), vec3(1.0, 1.0, 1.0), 1425.);



    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(vec3(-3, 10, -16), 2, ivory));
    spheres.push_back(Sphere(vec3(-1.0, -1.5, -12), 2, glass));
    spheres.push_back(Sphere(vec3( 1.5, -0.5, -18), 3, mirror));
    spheres.push_back(Sphere(vec3( 7,    5,   -18), 4,      mirror));
    spheres.push_back(Sphere(vec3(-5, 1, -10), 2, red_rubber));
    spheres.push_back(Sphere(vec3(-7, -5, -10), 1, turquiose));
    spheres.push_back(Sphere(vec3(-15,    -10,   -16), 2,      red_rubber));
    spheres.push_back(Sphere(vec3(-3,    0,   -16), 2,      ivory));
    // spheres.push_back(Sphere(vec3(-1.0, -1.5, -12), 2,      mirror));
    // spheres.push_back(Sphere(vec3( 1.5, -0.5, -18), 3, red_rubber));
    // spheres.push_back(Sphere(vec3( 7,    5,   -18), 4,     mirror));
    
    std::vector<Light> lights;
    lights.push_back(Light(vec3(-20,20,20),1.5));
    lights.push_back(Light(vec3( 30, 50, -25), 1.8));
    lights.push_back(Light(vec3( 30, 20,  30), 1.7));
    lights.push_back(Light(vec3(20,20,20), 0.75));
    // lights.push_back(Light(vec3(-20, 20,  20), 1.5));
    // lights.push_back(Light(vec3( 30, 50, -25), 1.8));
    // lights.push_back(Light(vec3( 30, 20,  30), 1.7));
    render(spheres, lights);



    

    return 0;
}