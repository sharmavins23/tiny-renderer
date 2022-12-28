#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
#include <cmath>
#include <iostream>
#include <ostream>
#include <vector>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

// ===== Geometry functions ====================================================

// Create a line between (p0.x, p0.y) -> (p1.x, p1.y) on the image with color
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    // If this is a "steep" line, we treat it separately (to fill in dots
    // easily)
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }

    // Let's also fix missing lines by swapping so p0.x < p1.x always
    if (p0.x > p1.x) std::swap(p0, p1);

    // COmpute and place the values
    for (int x = p0.x; x <= p1.x; x++) {
        // This is a basic computation of y=mx+b
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = p0.y * (1. - t) + (p1.y * t);

        // Remember to flip the data for steep values
        if (steep)
            image.set(y, x, color);
        else
            image.set(x, y, color);
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    // Compute the cross product here
    Vec3f u = cross(s[0], s[1]);

    // u[2] is int; If 0, then triangle is degenerate (so check if NOT!)
    if (std::abs(u[2]) > 1e-2)
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);

    // If the triangle is degenerate, return negative coords (throwaway coords)
    return Vec3f(-1, 1, 1);
}

// Given three vertices t0, t1, t2, place a triangle on the image
void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    // Create our bounding boxes
    Vec2f bboxmin(std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(),
                  -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);

    // Iterate through each pixel, and simply place it if inside the triangle
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }

    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i][2] * bc_screen[i];
            if (zbuffer[int(P.x + P.y * width)] < P.z) {
                zbuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

// ===== Screen functions ======================================================

// Project a value from an original vertex to the screen position
Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x + 1.) * width / 2. + .5),
                 int((v.y + 1.) * height / 2. + .5), v.z);
}

// ===== Driver code ===========================================================

int main(int argc, char **argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head/african_head.obj");
    }

    // Create our image
    TGAImage image(width, height, TGAImage::RGB);

    // Create our zBuffer structure
    float *zBuffer = new float[width * height];
    // Clear out the buffer
    for (int i = width * height; i--;
         zBuffer[i] = -std::numeric_limits<float>::max()) {
    }

    // For each face, simply call the rasterize function
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);

        Vec3f pts[3];
        // Project the points onto the screen
        for (int i = 0; i < 3; i++)
            pts[i] = world2screen(model->vert(face[i]));

        // Then, simply cast the triangle (with a random color)
        triangle(pts, zBuffer, image,
                 TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }

    image.flip_vertically(); // Origin at left bottom corner of the image
    image.write_tga_file("img/colorful.tga");
    delete model;
    return 0;
}