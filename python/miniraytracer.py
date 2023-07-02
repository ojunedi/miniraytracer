import math
from geometry import vec3, Sphere, Ray


def render(sphere: Sphere) -> None:
    WIDTH = 1024
    HEIGHT = 768
    FOV = math.pi / 3

    # intializing image
    framebuffer = [vec3(0,0,0) for _ in range(WIDTH * HEIGHT)]
    for j in range(HEIGHT):
        for i in range(WIDTH):
            framebuffer[i+(j*WIDTH)] = vec3(j/float(HEIGHT), i/float(WIDTH), 0)


    # 
    for j in range(HEIGHT):
        for i in range(WIDTH):
            x = ((2*(i + 0.5)/WIDTH) - 1)*math.tan(FOV/2)*WIDTH/HEIGHT
            y = -((2*(j+ 0.5)/HEIGHT) - 1)*math.tan(FOV/2)
            dir = vec3(x,y,-1).normalize()
            framebuffer[i + (j*WIDTH)] = cast_ray(vec3(0,0,0), dir, sphere)


    with open("./OUT.ppm", 'a') as f:
        f.write("P3\n")
        f.write(f"{WIDTH} {HEIGHT} \n255\n")
        for i in range(HEIGHT):
            for j in range(WIDTH):
                x = str(255 * max(0, min(1, framebuffer[i].x)))
                y = str(255 * max(0, min(1, framebuffer[i].y)))
                z = str(255 * max(0, min(1, framebuffer[i].z)))
                s = x +  " " + y + " " + z + " "
                f.write(s)
            f.write("\n")


def cast_ray(orig: vec3, dir: vec3, sphere: Sphere):
    sphere_dist = 2147483647.0
    if (not sphere.ray_intersect(orig, dir, sphere_dist)):
        return vec3(0.2, 0.7, 0.8)
    
    return vec3(0.4,0.4,0.3)



def main():

    sphere = Sphere(vec3(-3,0,-16), 2)
    render(sphere)

    print(sphere.ray_intersect(vec3(0,0,0), vec3(-0.200719, 0.125544, -1), 2147483647.0))


    # print(cast_ray(vec3(0,0,0), vec3(-0.200719, 0.125544, -1), sphere))


if __name__ == "__main__":
    main()