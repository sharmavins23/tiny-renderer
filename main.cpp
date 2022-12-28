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

// Given three vertices t0, t1, t2, place a triangle on the image
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    // Shortcut - No degenerate triangles
    if (t0.y == t1.y && t0.y == t2.y) return;

    // Swap around to get shorter values earlier
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);

    // Compute the overall height of the triangle
    int totalHeight = t2.y - t0.y;

    for (int i = 0; i < totalHeight; i++) {
        bool secondHalf = i > (t1.y - t0.y) || t1.y == t0.y;
        int segmentHeight = secondHalf ? (t2.y - t1.y) : (t1.y - t0.y);
        float alpha = (float)i / totalHeight;
        float beta =
            (float)(i - (secondHalf ? (t1.y - t0.y) : 0)) / segmentHeight;

        // Compute the A and B sides of the polygon
        Vec2i A = t0 + (t2 - t0) * alpha;
        Vec2i B = secondHalf ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

        if (A.x > B.x) std::swap(A, B);

        for (int j = A.x; j <= B.x; j++) {
            image.set(j, t0.y + i, color); // Note: t0.y + i != A.y
        }
    }
}

// ===== Driver code ===========================================================

int main(int argc, char **argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);

    // Position the light within the scene
    Vec3f light_dir(0, 0, -1);
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] =
                Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
            world_coords[j] = v;
        }
        Vec3f n = (world_coords[2] - world_coords[0]) ^
                  (world_coords[1] - world_coords[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0) {
            triangle(screen_coords[0], screen_coords[1], screen_coords[2],
                     image,
                     TGAColor(intensity * 255, intensity * 255, intensity * 255,
                              255));
        }
    }

    image.flip_vertically(); // Origin at left bottom corner of the image
    image.write_tga_file("img/polygons.tga");
    delete model;
    return 0;
}