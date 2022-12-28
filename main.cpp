#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

// Create a line between (x0, y0) -> (x1, y1) on the image with the color
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    // If this is a "steep" line, we treat it separately (to fill in dots
    // easily)
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }

    // Let's also fix missing lines by swapping so x0 < x1 always
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    // Compute slopes
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2; // Optimizing out floating points
    int error2 = 0;

    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }

        error2 += derror2;
        // If the error is non-negligible, factor it in
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

int main(int argc, char **argv) {
    TGAImage image(100, 100, TGAImage::RGB);

    line(17, 20, 80, 40, image, white);
    line(20, 13, 40, 80, image, red);
    line(80, 40, 13, 20, image, red);

    image.flip_vertically(); // Origin at left bottom corner of the image
    image.write_tga_file("img/line.tga");
    return 0;
}