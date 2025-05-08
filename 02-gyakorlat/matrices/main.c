#include "matrix.h"
#include <stdio.h>

int main() {
    float a[3][3] = {
        {1.0f, -2.0f,  3.0f},
        {5.0f, -3.0f,  0.0f},
        {-2.0f,  1.0f, -4.0f}
    };
    
    float b[3][3];
    float c[3][3];

    // 1. Egységmátrix inicializálása
    printf("\nEgységmátrix:\n");
    init_identity_matrix(b);
    print_matrix(b);

    // 2. Skalárral való szorzás (2-vel)
    printf("\nSkalárszorzás (2-vel):\n");
    scalar_multiply(a, 2.0f);
    print_matrix(a);

    // 3. Mátrixszorzás (a * b = c)
    printf("\nMátrixszorzás (A * Egységmátrix):\n");
    multiply_matrices(a, b, c);
    print_matrix(c);

    // 4. Pont transzformáció (homogén koordináták)
    float point[3] = {2.0f, 3.0f, 1.0f};
    printf("\nEredeti pont: [%4.2f, %4.2f]\n", point[0], point[1]);
    transform_point(a, point);
    printf("Transzformált pont: [%4.2f, %4.2f]\n", point[0], point[1]);

    // 5. Skálázás
    printf("\nSkálázás (3x, 2x):\n");
    init_identity_matrix(b);
    scale(b, 3.0f, 2.0f);
    print_matrix(b);

    // 6. Eltolás
    printf("\nEltolás (+5, -2):\n");
    shift(b, 5.0f, -2.0f);
    print_matrix(b);

    // 7. Forgatás (45 fok)
    printf("\nForgatás (45 fok):\n");
    rotate(b, 45.0f);
    print_matrix(b);

    return 0;
}
