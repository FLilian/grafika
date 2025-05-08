#include <stdio.h>
#include <stdbool.h>

typedef struct {
    double length;
    double width;
    double height;
} Cuboid;

void set_size(Cuboid *c, double l, double w, double h) {
    if (l > 0 && w > 0 && h > 0) {
        c->length = l;
        c->width = w;
        c->height = h;
    } else {
        printf("Hibás méretek! Az értékeknek pozitívnak kell lenniük.\n");
    }
}

double calc_volume(Cuboid c) {
    return c.length * c.width * c.height;
}

double calc_surface(Cuboid c) {
    return 2 * (c.length * c.width + c.width * c.height + c.height * c.length);
}

bool has_square_face(Cuboid c) {
    return (c.length == c.width) || (c.width == c.height) || (c.height == c.length);
}

int main() {
    Cuboid box;
    set_size(&box, 3.0, 4.0, 5.0);
    
    printf("Térfogat: %.2f\n", calc_volume(box));
    printf("Felszín: %.2f\n", calc_surface(box));

    if (has_square_face(box)) {
        printf("A téglatestnek van négyzet alakú lapja.\n");
    } else {
        printf("A téglatestnek nincs négyzet alakú lapja.\n");
    }

    return 0;
}
