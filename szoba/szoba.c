#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdbool.h>
#include <math.h>
#include <GL/glext.h>
#include <stdio.h>    
#include <SDL2/SDL_image.h>  
#include <GL/glut.h>
#include "model.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>


GLuint floorTexture;
void loadTextures();  // előzetes deklaráció
GLuint doorTexture;
void loadDoorTexture();
GLuint paintingTexture;
void loadPaintingTexture();
Model tree;
Model szek;
Model agy_szines;
Model ablak;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int doorOpen = 0; // 0 = zárva, 1 = nyitva
bool paintingSelected = false; // globális változó
float doorAngle = 90.0f; // Ajtó forgási szög


float camX = 0.0f, camY = 0.0f, camZ = 3.0f;
float camYaw = 0.0f;

float lightPos[] = { 0.0f, 0.5f, 4.95f, 1.0f }; // A lámpa helye
float lightIntensity = 1.0f; // Alap fényerő

float doorX = 5.5f - 0.001f; // A jobb fal közepére igazítjuk
float doorY = -1.0f; // Az ajtó alsó része a földön legyen
float doorZ = 0.4f; // A Z-tengelyen az ajtó középen legyen

float door_width = 2.0f;
float door_height = 3.5f;
float door_thickness = 0.1f;

float width = 1.5f;
float height = 1.0f;

float paintx = -5.0f + 0.01f;  // Bal fal síkja, kicsit beljebb tolva Z-fighting elkerülése érdekében
float painty = 0.0f;           // Középmagasságban
float paintz = 1.0f;           // A fal közepére

const char* help_text =
    "=== Használati útmutató ===\n"
    "A szoba tartalma:\n"
    " - Egy ágy, egy kép a falon, egy ajtó.\n"
    " - A ház felett egy repülő műhold található.\n"
    "\n"
    "Mit lehet csinálni:\n"
    " - A képet és az ajtót lehet mozgatni.\n"
    " - Ki lehet menni a szobából, és vissza is lehet térni.\n"
    " - A fényerőt lehet állítani a '-' és '+' billentyűkkel.\n"
    " - A kép a falon szintén mozgatható.\n"
    "\n"
    "Nyomj meg egy billentyűt a kilépéshez.";


#define CLOUDS_PER_WALL 6

float backOffsets[CLOUDS_PER_WALL];
float frontOffsets[CLOUDS_PER_WALL];
float leftOffsets[CLOUDS_PER_WALL];
float rightOffsets[CLOUDS_PER_WALL];

int cloudOffsetsInitialized = 0;


float birdAngle = 0.0f; // forgási szög
float birdRadius = 40.0f; // a körút sugara
float birdHeight = 25.0f; // magasság (Y)
typedef enum { FACE_FRONT_BACK, FACE_LEFT_RIGHT } WallFace;

void setCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0, (double)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float camDirX = sinf(camYaw);
    float camDirZ = -cosf(camYaw);

    gluLookAt(
        camX, camY, camZ,
        camX + camDirX, camY, camZ + camDirZ,
        0.0f, 1.0f, 0.0f
    );
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Csak az alap fény tulajdonságokat állítjuk itt be
    GLfloat lightColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);

    // Anyag beállítások
    GLfloat mat_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 50.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
}

void loadTextures() {
    SDL_Surface* surface = SDL_LoadBMP("wood.bmp"); // csak ez kell!
    if (!surface) {
        printf("Nem sikerült betölteni a textúrát: %s\n", SDL_GetError());
        return;
    }

    glGenTextures(1, &floorTexture);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0,
                 GL_BGR_EXT, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(surface);
}


void loadDoorTexture() {
    // Az IMG_Load függvény használata, amely támogatja a különböző képeket (pl. BMP, PNG, JPEG)
    SDL_Surface* surface = IMG_Load("door.png");
    if (!surface) {
        printf("Nem sikerült betölteni az ajtó textúrát: %s\n", SDL_GetError());
        return;
    }

    glGenTextures(1, &doorTexture);
    glBindTexture(GL_TEXTURE_2D, doorTexture);
    GLenum format;
    if (surface->format->BytesPerPixel == 4) { // Ha van alpha csatorna
        format = GL_RGBA;
    } else { // Ha nincs alpha
        format = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    SDL_FreeSurface(surface);
}


void loadPaintingTexture() {
    SDL_Surface* surface = IMG_Load("festmeny.png");
    if (!surface) {
        printf("Nem sikerült betölteni a festmény textúrát: %s\n", SDL_GetError());
        return;
    }

    glGenTextures(1, &paintingTexture);
    glBindTexture(GL_TEXTURE_2D, paintingTexture);

    GLenum format = (surface->format->BytesPerPixel == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format,
                 surface->w, surface->h, 0,
                 format, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    SDL_FreeSurface(surface);
}

void drawDoor() {
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, doorTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Anyag tulajdonságok
    glMaterialfv(GL_FRONT, GL_AMBIENT, (GLfloat[]){1.0f, 1.0f, 1.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat[]){1.0f, 1.0f, 1.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_EMISSION, (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});

    // Ajtó pozíció és forgatás a sarka körül
    glTranslatef(doorX, doorY, doorZ); // ajtó elhelyezés
    glTranslatef(-door_width / 4.0f, 0.0f, 0.0f); // eltolás a sarka felé
    glRotatef(doorAngle, 0.0f, 1.0f, 0.0f); // forgatás Y tengely körül
    glTranslatef(door_width / 4.0f, 0.0f, 0.0f); // vissza

    // Ajtó lap kirajzolása
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-door_width / 4, 0, 0);
    glTexCoord2f(1, 0); glVertex3f( door_width / 4, 0, 0);
    glTexCoord2f(1, 1); glVertex3f( door_width / 4, 1.8, 0);
    glTexCoord2f(0, 1); glVertex3f(-door_width / 4, 1.8, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


void drawPainting() {

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, paintingTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Anyag tulajdonságok
    glMaterialfv(GL_FRONT, GL_AMBIENT, (GLfloat[]){1.0f, 1.0f, 1.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat[]){1.0f, 1.0f, 1.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_EMISSION, (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});

    glTranslatef(paintx, painty, paintz); // Festmény helyzete a falon
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(0.0f,        -height/2, -width/2);
    glTexCoord2f(1, 1); glVertex3f(0.0f,        -height/2,  width/2);
    glTexCoord2f(1, 0); glVertex3f(0.0f,         height/2,  width/2);
    glTexCoord2f(0, 0); glVertex3f(0.0f,         height/2, -width/2);
    glEnd();


    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


void drawRoom() {
    glEnable(GL_TEXTURE_2D);

    // Padló - textúrázott
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBegin(GL_QUADS);
    glColor3f(1, 1, 1); // fehér, hogy ne színezze el a textúrát
    glTexCoord2f(0, 0); glVertex3f(-5, -1, -5);
    glTexCoord2f(1, 0); glVertex3f(5, -1, -5);
    glTexCoord2f(1, 1); glVertex3f(5, -1, 5);
    glTexCoord2f(0, 1); glVertex3f(-5, -1, 5);
    glEnd();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_2D);  // Ne használjunk textúrát

    // Plafon - barna
    glColor3f(0.73f, 0.33f, 0.83f); // barna szín
    glBegin(GL_QUADS);
    glVertex3f(-5, 1, -5);
    glVertex3f(5, 1, -5);
    glVertex3f(5, 1, 5);
    glVertex3f(-5, 1, 5);
    glEnd();

    // Hátsó fal - ablakhely kivágással
    glColor3f(1.0f, 0.75f, 0.8f); // rózsaszín fal
    glBegin(GL_QUADS);

    // Bal oldali rész az ablak mellett (most kisebb és eltolva)
    glVertex3f(-5, -1, 5);
    glVertex3f(-3.4f, -1, 5);   // Bal oldalon, ahol az ablak kezdődik
    glVertex3f(-3.4f, 1, 5);    // A bal szél az ablak textúrája mellett
    glVertex3f(-5, 1, 5);

    // Jobb oldali rész az ablak mellett (jobbra tolva)
    glVertex3f(-2.4f, -1, 5);    // Jobb oldalra tolva, az ablak textúrája mellett
    glVertex3f(5, -1, 5);
    glVertex3f(5, 1, 5);
    glVertex3f(-2.4f, 1, 5);     // Jobbra tolva

    // Felső rész az ablak felett
    glVertex3f(-3.4f, 0.5f, 5);    // Az ablak tetejétől
    glVertex3f(-2.4f, 0.5f, 5);     // Felső rész igazítása a jobb oldalra
    glVertex3f(-2.4f, 1.0f, 5);     // Felső rész igazítása a jobb oldalra
    glVertex3f(-3.4f, 1.0f, 5);    // Felső rész igazítása a bal oldalra

    // Alsó rész az ablak alatt
    glVertex3f(-3.4f, -1.0f, 5);   // Az ablak alja
    glVertex3f(-2.4f, -1.0f, 5);    // Alsó rész igazítása a jobb oldalra
    glVertex3f(-2.4f, -0.51f, 5);    // Alsó rész igazítása a jobb oldalra
    glVertex3f(-3.4f, -0.51f, 5);   // Alsó rész igazítása a bal oldalra

    glEnd();
   
    // Barna keret körül az ablak kivágásának
    glColor3f(0.6f, 0.3f, 0.0f);  // Barna szín
    glBegin(GL_QUADS);

    float keretVastagsag = 0.1f;  // Általános keretvastagság

    // Bal oldali keret
    glVertex3f(-3.2f - keretVastagsag, -0.4f, 5);
    glVertex3f(-3.4f, -0.4f, 5);
    glVertex3f(-3.4f, 0.4f, 5);
    glVertex3f(-3.2f - keretVastagsag, 0.4f, 5);

    // Jobb oldali keret
    glVertex3f(-2.4f, -0.4f, 5);
    glVertex3f(-2.6f + keretVastagsag, -0.4f, 5);
    glVertex3f(-2.6f + keretVastagsag, 0.4f, 5);
    glVertex3f(-2.4f, 0.4f, 5);

    // Felső keret
    glVertex3f(-3.4f, 0.6f, 5);
    glVertex3f(-2.4f, 0.6f, 5);
    glVertex3f(-2.4f, 0.3f + keretVastagsag, 5);
    glVertex3f(-3.4f, 0.3f + keretVastagsag, 5);

    // Alsó keret
    glVertex3f(-3.4f, -0.3f - keretVastagsag, 5);
    glVertex3f(-2.4f, -0.3f - keretVastagsag, 5);
    glVertex3f(-2.4f, -0.6f, 5);
    glVertex3f(-3.4f, -0.6f, 5);

    glEnd();

    glColor3f(1.0f, 0.75f, 0.8f); // rózsaszín fal
    glBegin(GL_QUADS);
    // első
    glVertex3f(-5, -1, -5);
    glVertex3f(5, -1, -5);
    glVertex3f(5, 1, -5);
    glVertex3f(-5, 1, -5);
    // bal
    glVertex3f(-5, -1, -5);
    glVertex3f(-5, -1, 5);
    glVertex3f(-5, 1, 5);
    glVertex3f(-5, 1, -5);
    // jobb
    // Jobb oldali fal – AJTÓVAL
    glColor3f(0.8f, 0.7f, 0.6f); // világos barna fal
    glBegin(GL_QUADS);
    // Ajtó előtti rész (bal oldalon)
    glVertex3f(5, -1, -5);
    glVertex3f(5, -1, -0.599f);
    glVertex3f(5, 1, -0.599f);
    glVertex3f(5, 1, -5);

    // Ajtó utáni rész (jobb oldalon)
    glVertex3f(5, -1, 0.4f);
    glVertex3f(5, -1, 5);
    glVertex3f(5, 1, 5);
    glVertex3f(5, 1, 0.4f);

    // Ajtó fölötti rész
    glVertex3f(5, 0.799f, -0.599f);
    glVertex3f(5, 0.799f, 0.4f);
    glVertex3f(5, 1.0f, 0.4f);
    glVertex3f(5, 1.0f, -0.599f);
    glEnd();
    

    // Fekete kontúrok – jobb fal két szakaszára külön
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex3f(5, -1, -5);
    glVertex3f(5, -1, -0.599f);
    glVertex3f(5, 1, -0.599f);
    glVertex3f(5, 1, -5);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f(5, -1, 0.4f);
    glVertex3f(5, -1, 5);
    glVertex3f(5, 1, 5);
    glVertex3f(5, 1, 0.4f);
    glEnd();


    glEnable(GL_TEXTURE_2D);

    // Fekete keretek kis eltolással
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-1.0, -1.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    glColor3f(0, 0, 0);

    glBegin(GL_QUADS);
    // ugyanazok a pontok, mint fent
    // padló
    glVertex3f(-5, -1, -5);
    glVertex3f(5, -1, -5);
    glVertex3f(5, -1, 5);
    glVertex3f(-5, -1, 5);
    // plafon
    glVertex3f(-5, 1, -5);
    glVertex3f(5, 1, -5);
    glVertex3f(5, 1, 5);
    glVertex3f(-5, 1, 5);
    // falak (ugyanaz mint fent)
    glVertex3f(-5, -1, 5);
    glVertex3f(5, -1, 5);
    glVertex3f(5, 1, 5);
    glVertex3f(-5, 1, 5);
    glVertex3f(-5, -1, -5);
    glVertex3f(5, -1, -5);
    glVertex3f(5, 1, -5);
    glVertex3f(-5, 1, -5);
    glVertex3f(-5, -1, -5);
    glVertex3f(-5, -1, 5);
    glVertex3f(-5, 1, 5);
    glVertex3f(-5, 1, -5);
    glVertex3f(5, -1, -5);
    glVertex3f(5, -1, 5);
    glVertex3f(5, 1, 5);
    glVertex3f(5, 1, -5);
    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
}



void drawOvalOnWall(float cx, float cy, float radiusX, float radiusY, float depth, WallFace face) {
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i <= 30; i++) {
        float angle = 2.0f * M_PI * i / 30;
        float x = cosf(angle) * radiusX;
        float y = sinf(angle) * radiusY;

        if (face == FACE_FRONT_BACK) {
            glVertex3f(cx + x, cy + y, depth);  // Z fal
        } else {
            glVertex3f(depth, cy + y, cx + x);  // X fal (jobb/bal)
        }
    }
    glEnd();
}


void drawCloud(float cx, float cy, float depth, float scaleX, float scaleY, WallFace face) {
    glColor3f(1.0f, 1.0f, 1.0f);
    float offset = scaleX * 0.6f;

    drawOvalOnWall(cx,     cy, scaleX, scaleY, depth, face);
    drawOvalOnWall(cx - offset, cy, scaleX, scaleY, depth, face);
    drawOvalOnWall(cx + offset, cy, scaleX, scaleY, depth, face);
    drawOvalOnWall(cx - offset * 0.5f, cy + offset * 0.5f, scaleX, scaleY, depth, face);
    drawOvalOnWall(cx + offset * 0.5f, cy + offset * 0.5f, scaleX, scaleY, depth, face);
}

void drawTriangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
    glBegin(GL_TRIANGLES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glEnd();
}

void drawSphere(float x, float y, float z, float radius, bool iswhite) {
    GLUquadric* quad = gluNewQuadric();
    glPushMatrix();
    glTranslatef(x, y, z);  // Elhelyezi a gömböt az x, y, z koordinátákra

    if (iswhite) {
        glColor3f(1.0f, 1.0f, 1.0f);  // Fehér szín
    } else {
        glColor3f(0.0f, 0.0f, 0.0f);  // Fekete szín
    }
    
    gluSphere(quad, radius, 20, 20);  // Egy 3D gömb rajzolása
    glPopMatrix();
}

void drawBird(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);  // Elhelyezi a madarat az x, y, z koordinátákra
    
    // Test (nagyobb gömb)
    drawSphere(0.0f, 0.0f, 0.0f, 1.0f, false);  // A test gömbjének elhelyezése
    glColor3f(0.0f, 0.0f, 0.0f);  // Fekete szín
    // Szárnyak (háromszögek)
    // Bal szárny (háromszög)
    drawTriangle(
        -1.0f,  0.0f,  0.0f,  // A: találkozási pont a testtel
        -2.0f,  1.8f,  0.0f,  // B: felső külső sarok
        -2.0f, -0.8f,  0.0f   // C: alsó külső sarok
    );
     // Bal szárny háromszög, csúcs előre néz
    
    // Jobb szárny (háromszög)
    drawTriangle(
        0.8f,  0.0f,  0.0f,   // A: találkozási pont a testtel
        1.8f,  1.8f,  0.0f,   // B: felső külső sarok
        1.8f, -0.8f,  0.0f    // C: alsó külső sarok
    );  // Jobb szárny háromszög, csúcs előre néz

    glPopMatrix();  // Madár befejezése
}



void drawOutsideWorld() {
    glDisable(GL_LIGHTING);  // ne hasson rá fény
    glDisable(GL_TEXTURE_2D);


    if (!cloudOffsetsInitialized) {
        srand((unsigned int)time(NULL));
        for (int i = 0; i < CLOUDS_PER_WALL; ++i) {
            backOffsets[i] = ((rand() % 100) / 100.0f - 0.5f) * 10.0f;
            frontOffsets[i] = ((rand() % 100) / 100.0f - 0.5f) * 10.0f;
            leftOffsets[i] = ((rand() % 100) / 100.0f - 0.5f) * 10.0f;
            rightOffsets[i] = ((rand() % 100) / 100.0f - 0.5f) * 10.0f;
        }
        cloudOffsetsInitialized = 1;
    }
    

    float s = 50.0f; // a világ mérete
    float h = -1.5f;  // alsó Y szint, lejjebb mint a szoba padlója

    // Égbolt (kék) – oldalak és plafon
    glColor3f(0.4f, 0.7f, 1.0f); 

    // Hátsó fal
    glBegin(GL_QUADS);
    glVertex3f(-s, h, -s);
    glVertex3f( s, h, -s);
    glVertex3f( s, s, -s);
    glVertex3f(-s, s, -s);
    glEnd();

    // Elülső fal
    glBegin(GL_QUADS);
    glVertex3f(-s, h, s);
    glVertex3f( s, h, s);
    glVertex3f( s, s, s);
    glVertex3f(-s, s, s);
    glEnd();

    // Bal fal
    glBegin(GL_QUADS);
    glVertex3f(-s, h, -s);
    glVertex3f(-s, h, s);
    glVertex3f(-s, s, s);
    glVertex3f(-s, s, -s);
    glEnd();

    // Jobb fal
    glBegin(GL_QUADS);
    glVertex3f(s, h, -s);
    glVertex3f(s, h, s);
    glVertex3f(s, s, s);
    glVertex3f(s, s, -s);
    glEnd();

    // Plafon
    glBegin(GL_QUADS);
    glVertex3f(-s, s, -s);
    glVertex3f(s, s, -s);
    glVertex3f(s, s, s);
    glVertex3f(-s, s, s);
    glEnd();

    // Fű (zöld)
    glColor3f(0.2f, 0.8f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-s, h, -s);
    glVertex3f(s, h, -s);
    glVertex3f(s, h, s);
    glVertex3f(-s, h, s);
    glEnd();

    // Felhők
    glColor3f(1.0f, 1.0f, 1.0f);
    float cloudY = 20.0f; // felhők magassága
    float cloudScaleX = 2.0f;
    float cloudScaleY = 1.0f;
    
    float margin = 5.0f;
    float spacing = (2 * s - 2 * margin) / (CLOUDS_PER_WALL - 1);
     // Hátsó fal (-Z)
    for (int i = 0; i < CLOUDS_PER_WALL; ++i) {
        float x = -s + margin + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(x, cloudY + backOffsets[i], -s + 0.01f, cloudScaleX, cloudScaleY, FACE_FRONT_BACK);
    }

    for (int i = 0; i < 3; ++i) {
        float x = -s + margin+8 + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(x, cloudY + backOffsets[i]+15, -s + 0.01f, cloudScaleX, cloudScaleY, FACE_FRONT_BACK);
    }

    // Elülső fal (+Z)
    for (int i = 0; i < CLOUDS_PER_WALL; ++i) {
        float x = -s + margin + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(x, cloudY + frontOffsets[i], s - 0.01f, cloudScaleX, cloudScaleY, FACE_FRONT_BACK);
    }

    for (int i = 0; i < 3; ++i) {
        float x = -s + margin+8 + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(x, cloudY + frontOffsets[i]+16, s - 0.01f, cloudScaleX, cloudScaleY, FACE_FRONT_BACK);
    }
    // Bal fal (-X)
    for (int i = 0; i < CLOUDS_PER_WALL; ++i) {
        float z = -s + margin + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(z, cloudY + leftOffsets[i], -s + 0.01f, cloudScaleX, cloudScaleY, FACE_LEFT_RIGHT);
    }

    for (int i = 0; i < 3; ++i) {
        float z = -s + margin+8 + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(z, cloudY + leftOffsets[i]+10, -s + 0.01f, cloudScaleX, cloudScaleY, FACE_LEFT_RIGHT);
    }

    // Jobb fal (+X)
    for (int i = 0; i < CLOUDS_PER_WALL; ++i) {
        float z = -s + margin + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(z, cloudY + rightOffsets[i], s - 0.01f, cloudScaleX, cloudScaleY, FACE_LEFT_RIGHT);
    }

        // Jobb fal (+X)
    for (int i = 0; i < 3; ++i) {
        float z = -s + margin+8 + spacing * i; // biztosítjuk, hogy ne menjen túl
        drawCloud(z, cloudY + rightOffsets[i]+13, s - 0.01f, cloudScaleX, cloudScaleY, FACE_LEFT_RIGHT);
    }

    birdAngle += 0.01f; // animációhoz: lassan növeljük a szöget

    // madár pozíciója: körben repül a falak belsejében
    float birdX = cosf(birdAngle) * birdRadius;
    float birdZ = sinf(birdAngle) * birdRadius;
    drawBird(birdX, birdHeight, birdZ);


    glEnable(GL_LIGHTING);
}


void draw_model(const Model* model) {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < model->face_count; i++) {
        Face f = model->faces[i];
        Vertex v1 = model->vertices[f.v1 - 1]; // OBJ index 1-alapú
        Vertex v2 = model->vertices[f.v2 - 1];
        Vertex v3 = model->vertices[f.v3 - 1];

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
}


void drawLightSource() {
    GLUquadric* quad = gluNewQuadric();

    glPushMatrix();
    glTranslatef(lightPos[0], lightPos[1], lightPos[2]);

    // ⚠️ Kapcsolj ki mindent, ami elrontja a színt
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    // 🔁 Reset material state ha előtte bármi más volt beállítva
    glMaterialfv(GL_FRONT, GL_AMBIENT, (GLfloat[]){1.0f, 1.0f, 1.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat[]){1.0f, 1.0f, 1.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});
    glMaterialfv(GL_FRONT, GL_EMISSION, (GLfloat[]){0.0f, 0.0f, 0.0f, 1.0f});
    
    // 🌟 Fix szín
    glColor3f(1.0f, 1.0f, 0.7f);

    gluSphere(quad, 0.15, 20, 20);

    // ✅ Visszakapcsolunk mindent a következő objektumokhoz
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    glPopMatrix();
    gluDeleteQuadric(quad);
}


void show_help() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printf("%s\n", help_text);
    getchar(); // vár egy gombnyomásra
}
    


int load_model(const char* filename, Model* model) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;

    model->vertices = NULL;
    model->faces = NULL;
    model->vertex_count = 0;
    model->face_count = 0;

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            model->vertices = realloc(model->vertices, (model->vertex_count + 1) * sizeof(Vertex));
            sscanf(line, "v %f %f %f",
                   &model->vertices[model->vertex_count].x,
                   &model->vertices[model->vertex_count].y,
                   &model->vertices[model->vertex_count].z);
            model->vertex_count++;
        } else if (line[0] == 'f') {
            model->faces = realloc(model->faces, (model->face_count + 1) * sizeof(Face));
            sscanf(line, "f %d %d %d",
                   &model->faces[model->face_count].v1,
                   &model->faces[model->face_count].v2,
                   &model->faces[model->face_count].v3);
            model->face_count++;
        }
    }

    fclose(file);
    return 1;
}

void free_model(Model* model) {
    free(model->vertices);
    free(model->faces);
    model->vertices = NULL;
    model->faces = NULL;
    model->vertex_count = 0;
    model->face_count = 0;
}


bool rayIntersectsAABB(float rayOrigX, float rayOrigY, float rayOrigZ,
    float rayDirX, float rayDirY, float rayDirZ,
    float minX, float maxX, float minY, float maxY,
    float minZ, float maxZ) {
    float tmin = (minX - rayOrigX) / rayDirX;
    float tmax = (maxX - rayOrigX) / rayDirX;
    if (tmin > tmax) { float tmp = tmin; tmin = tmax; tmax = tmp; }

    float tymin = (minY - rayOrigY) / rayDirY;
    float tymax = (maxY - rayOrigY) / rayDirY;
    if (tymin > tymax) { float tmp = tymin; tymin = tymax; tymax = tmp; }

    if ((tmin > tymax) || (tymin > tmax)) return false;
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (minZ - rayOrigZ) / rayDirZ;
    float tzmax = (maxZ - rayOrigZ) / rayDirZ;
    if (tzmin > tzmax) { float tmp = tzmin; tzmin = tzmax; tzmax = tmp; }

    if ((tmin > tzmax) || (tzmin > tmax)) return false;

    return true;
}



 
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("3D Szoba", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);



    glEnable(GL_DEPTH_TEST);
    glClearColor(0.4f, 0.7f, 1.0f, 1.0f); // világoskék ég háttér

    loadTextures(); // FONTOS: kontextus után hívjuk

    if (!load_model("asztal.obj", &tree)) {
        fprintf(stderr, "Nem sikerült betölteni a fát!\n");
        return 1;
    }

    if (!load_model("szek.obj", &szek)) {
        fprintf(stderr, "Nem sikerült betölteni a fát!\n");
        return 1;
    }

    if (!load_model("agy_szines.obj", &agy_szines)) {
        fprintf(stderr, "Nem sikerült betölteni a rózsaszín ágyat!\n");
        return 1;
    }
    
    if (!load_model("ablak.obj", &ablak)) {
        fprintf(stderr, "Nem sikerült betölteni az ablakot!\n");
        return 1;
    }
    
    loadDoorTexture();
    loadPaintingTexture();
    initLighting();
    
    bool running = true;
    SDL_Event event;

    while (running) {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
                else if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_KP_PLUS)
                    lightIntensity += 0.1f;
                else if (event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_KP_MINUS)
                    lightIntensity -= 0.1f;
                else if (event.key.keysym.sym == SDLK_F1)
                    show_help();
                if (lightIntensity < 0.0f) lightIntensity = 0.0f;
                if (lightIntensity > 5.0f) lightIntensity = 5.0f;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
            
                // === 1. NDC koordináták számítása ===
                float ndcX = (2.0f * mouseX) / SCREEN_WIDTH - 1.0f;
                float ndcY = 1.0f - (2.0f * mouseY) / SCREEN_HEIGHT;
            
                // === 2. Ray irány számítás ===
                float fov = 70.0f;
                float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
                float tanFov = tanf(fov * 0.5f * M_PI / 180.0f);
            
                float rayDirCameraX = ndcX * aspect * tanFov;
                float rayDirCameraY = ndcY * tanFov;
                float rayDirCameraZ = -1.0f;
            
                float sinYaw = sinf(camYaw);
                float cosYaw = cosf(camYaw);
            
                float rayDirWorldX = rayDirCameraX * cosYaw - rayDirCameraZ * sinYaw;
                float rayDirWorldY = rayDirCameraY;
                float rayDirWorldZ = rayDirCameraX * sinYaw + rayDirCameraZ * cosYaw;
            
                float length = sqrtf(rayDirWorldX * rayDirWorldX +
                                     rayDirWorldY * rayDirWorldY +
                                     rayDirWorldZ * rayDirWorldZ);
                rayDirWorldX /= length;
                rayDirWorldY /= length;
                rayDirWorldZ /= length;
            
                // === Festmény: Ray-plane metszés az X = paintx síkkal ===
                if (fabs(rayDirWorldX) > 1e-6f) { // ne ossz nullával
                    float t = (paintx - camX) / rayDirWorldX;
            
                    if (t > 0.0f) {
                        float hitY = camY + t * rayDirWorldY;
                        float hitZ = camZ + t * rayDirWorldZ;
            
                        float halfH = height / 2.0f;
                        float halfW = width / 2.0f;
            
                        if (hitY >= painty - halfH && hitY <= painty + halfH &&
                            hitZ >= paintz - halfW && hitZ <= paintz + halfW) {
                            printf("Festmény eltalálva!\n");
                            paintingSelected = true;
                        }
                    }
                }
            
                // === Ajtó: AABB ellenőrzés ===
                if (rayIntersectsAABB(camX, camY, camZ,
                    rayDirWorldX, rayDirWorldY, rayDirWorldZ,
                    doorX - door_thickness / 2.0f, doorX + door_thickness / 2.0f,
                    doorY, doorY + door_height,
                    doorZ - door_width / 2.0f, doorZ + door_width / 2.0f)) {
                    printf("ajtó eltalálva!\n");
                    doorOpen = !doorOpen;
                }
            }
            

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                paintingSelected = false;
            }
        
            // ======= Egér mozgatása = festmény húzása =======
            if (event.type == SDL_MOUSEMOTION && paintingSelected) {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
        
                float ndcX = (2.0f * mouseX) / SCREEN_WIDTH - 1.0f;
                float ndcY = 1.0f - (2.0f * mouseY) / SCREEN_HEIGHT;
        
                float fov = 70.0f;
                float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
                float tanFov = tanf(fov * 0.5f * M_PI / 180.0f);
        
                float rayDirCameraX = ndcX * aspect * tanFov;
                float rayDirCameraY = ndcY * tanFov;
                float rayDirCameraZ = -1.0f;
        
                float sinYaw = sinf(camYaw);
                float cosYaw = cosf(camYaw);
        
                float rayDirWorldX = rayDirCameraX * cosYaw - rayDirCameraZ * sinYaw;
                float rayDirWorldY = rayDirCameraY;
                float rayDirWorldZ = rayDirCameraX * sinYaw + rayDirCameraZ * cosYaw;
        
                float length = sqrtf(rayDirWorldX * rayDirWorldX + rayDirWorldY * rayDirWorldY + rayDirWorldZ * rayDirWorldZ);
                rayDirWorldX /= length;
                rayDirWorldY /= length;
                rayDirWorldZ /= length;
        
                // Metszés a bal fal síkjával (X = paintx)
                float t = (paintx - camX) / rayDirWorldX;
                if (t > 0.0f) {
                    float intersectZ = camZ + t * rayDirWorldZ;
        
                    float halfWidth = width / 2.0f;
                    float minZ = -5.0f + halfWidth;
                    float maxZ = 5.0f - halfWidth;
        
                    if (intersectZ >= minZ && intersectZ <= maxZ) {
                        paintz = intersectZ;
                    }
                }
            }
        
            
        }

        if (!doorOpen && doorAngle < 90.0f) {
            doorAngle += 1.5f;
            if (doorAngle > 90.0f) doorAngle = 90.0f;
        } else if (doorOpen && doorAngle > 0.0f) {
            doorAngle -= 1.5f;
            if (doorAngle < 0.0f) doorAngle = 0.0f;
        }

        // Mozgási sebesség
        float camSpeed = 0.1f;
        float nextX = camX;
        float nextZ = camZ;

        // Kamera forgatás (mindig engedélyezett)
        if (keystate[SDL_SCANCODE_LEFT])
            camYaw -= 0.05f;
        if (keystate[SDL_SCANCODE_RIGHT])
            camYaw += 0.05f;

        // Mozgási irány
        if (keystate[SDL_SCANCODE_UP]) {
            nextX += sinf(camYaw) * camSpeed;
            nextZ += -cosf(camYaw) * camSpeed;
        }
        if (keystate[SDL_SCANCODE_DOWN]) {
            nextX -= sinf(camYaw) * camSpeed;
            nextZ -= -cosf(camYaw) * camSpeed;
        }

        // Szoba határok (kicsit beljebb a fal miatt)
        float wallThickness = 0.2f;
        float roomMinX = -5.0f + wallThickness;
        float roomMaxX =  5.0f - wallThickness;
        float roomMinZ = -5.0f + wallThickness;
        float roomMaxZ =  5.0f - wallThickness;


        // Jelenlegi helyzet
        bool currentlyInsideRoom = camX >= roomMinX && camX <= roomMaxX &&
                                camZ >= roomMinZ && camZ <= roomMaxZ;

        bool nextInsideRoom = nextX >= roomMinX && nextX <= roomMaxX &&
                            nextZ >= roomMinZ && nextZ <= roomMaxZ;

        // Kamera az ajtó felé néz?
        float dirToDoorX = doorX - camX;
        float dirToDoorZ = doorZ - camZ;
        float angleToDoor = atan2f(dirToDoorZ, dirToDoorX);
        float angleDiff = fabs(camYaw - angleToDoor);
        if (angleDiff > M_PI) angleDiff = 2.0f * M_PI - angleDiff;
        bool lookingAtDoor = angleDiff < 0.8726646f; // 50 fok

        float distanceToDoor = sqrtf((camX - doorX) * (camX - doorX) +
                             (camZ - doorZ) * (camZ - doorZ));

        bool standingAtDoor = distanceToDoor < 1.5f;  // 1.5 elég jó indulásnak
                                 
        // Mozgási irány az ajtón kívülre vezet?
        bool nextOutsideRoom = !(nextX >= roomMinX && nextX <= roomMaxX &&
                                nextZ >= roomMinZ && nextZ <= roomMaxZ);

        // Végső engedélyezés logika
        bool allowMovement = false;

        if (!currentlyInsideRoom) {
            allowMovement = true;  // már kint vagy, bármerre mehetsz
        } else {
            if (!doorOpen) {
                if (nextInsideRoom) {
                    allowMovement = true;  // ajtó zárva, de a szobán belül maradsz
                }
            } else {
                // Ajtó nyitva
                if (lookingAtDoor && standingAtDoor && nextOutsideRoom) {
                    allowMovement = true;  // kijárás az ajtón keresztül
                } else if (nextInsideRoom) {
                    allowMovement = true;  // a szobán belül mozogsz
                }
            }
        }

        // Mozgás alkalmazása
        if (allowMovement) {
            camX = nextX;
            camZ = nextZ;
        }


                        

        // renderelés
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        setCamera(); // először kamera
        drawOutsideWorld();

        // most állítsd be a fény pozícióját a gömb pozíciójához
        GLfloat lightPos[] = { 2.0f, 2.0f, 2.0f, 2.0f }; // vagy ahol a gömb lebeg
        GLfloat lightColor[] = {
            1.0f * lightIntensity,
            1.0f * lightIntensity,
            0.7f * lightIntensity,
            1.0f
        };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);

        drawRoom();
        
        
        glPushMatrix();
        glTranslatef(-4.48f, -0.49f, -4.48f);  // asztal eltolása
        glScalef(0.1f, 0.1f, 0.1f);      // asztal méretezése
        draw_model(&tree);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-3.6f, -0.7f, -4.4f);  // szék eltolása
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);  // szék elforgatása jobbra (Y tengely körül)
        glScalef(0.02f, 0.02f, 0.02f);       // szék méretezése
        draw_model(&szek);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(-3.6f, -1.0f, 4.4f);  // agy eltolása
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);  // agy elforgatása jobbra (Y tengely körül)
        glScalef(0.55f, 0.55f, 0.55f);       // agy méretezése
        draw_model(&agy_szines);
        glPopMatrix();

        // Ablak rajzolása
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.5f, 0.8f, 1.0f, 0.1f);  // Világoskék, áttetsző

        glPushMatrix();
        glTranslatef(-2.9f, -0.5f, 4.99f);  // Falra téve, például Z = -5
        draw_model(&ablak);
        glPopMatrix();

        glColor4f(1, 1, 1, 1);  // Visszaállítás
        glDisable(GL_BLEND);

        drawDoor(); 
        drawLightSource(); // ha ez a gömb, ami jelképezi a fényt
        drawPainting(); 

        SDL_GL_SwapWindow(window);
        SDL_Delay(16);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}



