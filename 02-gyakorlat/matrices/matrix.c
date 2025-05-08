#include "matrix.h"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>


void init_identity_matrix(float matrix[3][3]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            matrix[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

void scalar_multiply(float matrix[3][3], float scalar) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            matrix[i][j] *= scalar;
        }
    }
}

void multiply_matrices(const float a[3][3], const float b[3][3], float c[3][3]) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            c[i][j] = 0.0f;
            for (int k = 0; k < 3; ++k) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void transform_point(const float matrix[3][3], float point[3]) {
    float result[3] = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[i] += matrix[i][j] * point[j];
        }
    }
    for (int i = 0; i < 3; ++i) {
        point[i] = result[i];
    }
}

void scale(float matrix[3][3], float sx, float sy) {
    float scale_matrix[3][3] = {
        {sx,  0,  0},
        { 0, sy,  0},
        { 0,  0,  1}
    };
    multiply_matrices(matrix, scale_matrix, matrix);
}

void shift(float matrix[3][3], float dx, float dy) {
    float shift_matrix[3][3] = {
        {1, 0, dx},
        {0, 1, dy},
        {0, 0,  1}
    };
    multiply_matrices(matrix, shift_matrix, matrix);
}

void rotate(float matrix[3][3], float angle) {
    float rad = angle * (M_PI / 180.0f);
    float cosA = cosf(rad);
    float sinA = sinf(rad);
    float rotate_matrix[3][3] = {
        {cosA, -sinA, 0},
        {sinA,  cosA, 0},
        {   0,     0, 1}
    };
    multiply_matrices(matrix, rotate_matrix, matrix);
}

void init_zero_matrix(float matrix[3][3])
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            matrix[i][j] = 0.0;
        }
    }
}

void print_matrix(const float matrix[3][3])
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            printf("%4.4f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void add_matrices(const float a[3][3], const float b[3][3], float c[3][3])
{
    int i;
    int j;

    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            c[i][j] = a[i][j] + b[i][j];
        }
    }
}

