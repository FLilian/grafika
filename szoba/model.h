#ifndef MODEL_H
#define MODEL_H

#define MAX_VERTICES 100
#define MAX_FACES 100

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float u, v;
} TexCoord;

typedef struct {
    int v1, v2, v3;
    int vt1, vt2, vt3;  // textúra koordináta indexek
} Face;

typedef struct {
    GLuint texture_id;    // OpenGL textúra ID
    char name[64];        // anyag neve
} Material;

typedef struct {
    Vertex* vertices;
    TexCoord* texcoords;
    Face* faces;
    Material material;
    int vertex_count;
    int texcoord_count;
    int face_count;
} Model;

int load_model(const char* filename, Model* model);
void free_model(Model* model);

#endif
