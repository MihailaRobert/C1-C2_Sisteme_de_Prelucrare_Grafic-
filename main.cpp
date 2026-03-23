#include <freeglut.h> 
#include <cmath>
#include <vector> //P3_c_2STALPI

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int width = 800;
static int height = 600;


// P3_a: VARIABILE CONTROL CAMERA
float camX = 0.0f, camY = 2.0f, camZ = 50.0f;
float rotX = 15.0f, rotY = 0.0f;


// **_C1_a: VARIABILE CONTROL OBIECT (MAȘINĂ)
float objX = 2.0f, objZ = 40.0f; // Poziția inițială pe drum 
float objRot = 0.0f;             // Rotația obiectului
float objSpeed = 0.4f;           // Viteza de deplasare




// ****_C2_a: VARIABILE PENTRU PĂSĂRI (Zbor continuu, nu oscilatoriu)
float birdX[3] = { -10.0f, 5.0f, 15.0f };
float birdZ[3] = { 10.0f, -5.0f, 20.0f };
float birdY[3] = { 12.0f, 15.0f, 10.0f };
float birdDir[3] = { 0.0f, 120.0f, 240.0f }; // Direcția inițială în grade
float birdAngle[3] = { 0.0f, 0.0f, 0.0f };   // Pentru rotația modelului
float wingAngle = 0.0f;


// ****_C2_b: VARIABILE PENTRU MAȘINI AUTOMATE (2 MAȘINI)
float autoAngle[2] = { 0.0f, 3.14159f }; // Poziții opuse pe cerc
float autoRadius = 16.0f;               // Raza circuitului (rC)
float autoSpeed = 0.01f;                // Viteza de rotație







// **_C1_b: STRUCTURA PENTRU COLIZIUNI (Bounding Box simplificat) 
struct BoundingBox {
    float minX, maxX, minZ, maxZ;
};

// Listă de obstacole (clădiri) pentru verificarea coliziunilor
std::vector<BoundingBox> obstacole;

// Funcție pentru a adăuga obstacole (se apelează în main sau unde se cunosc pozițiile clădirilor)
void adaugaObstacol(float x, float z, float dimensiune) {
    BoundingBox b;
    b.minX = x - dimensiune; b.maxX = x + dimensiune;
    b.minZ = z - dimensiune; b.maxZ = z + dimensiune;
    obstacole.push_back(b);
}




// **_C1_b: FUNCȚIE DETECTARE COLIZIUNI 
bool verificaColiziune(float viitorX, float viitorZ) {
    float razaObiect = 0.6f; // Dimensiunea aproximativă a mașinii
    for (const auto& obs : obstacole) {
        if (viitorX + razaObiect > obs.minX && viitorX - razaObiect < obs.maxX &&
            viitorZ + razaObiect > obs.minZ && viitorZ - razaObiect < obs.maxZ) {
            return true; // Coliziune detectată
        }
    }
    return false;
}

// **_C1_b: Verifică dacă mașina jucătorului se lovește de mașinile automate
bool verificaColiziuneMasini(float viitorX, float viitorZ) {
    float distantaMinima = 3.0f; 

    for (int i = 0; i < 2; i++) {
        // Calculăm poziția actuală a mașinii automate (folosind datele din display)
        float autoX = 2.0f + 16.0f * cos(autoAngle[i]);
        float autoZ = -2.0f + 16.0f * sin(autoAngle[i]);

        // Calculăm distanța euclidiană (Aritmetica vectorilor)
        float dist = sqrt(pow(viitorX - autoX, 2) + pow(viitorZ - autoZ, 2));

        if (dist < distantaMinima) {
            return true; // Coliziune detectată
        }
    }
    return false;
}




// **_****_C1_b & C2_b: Verifică distanța între două puncte arbitrare (pentru orice mașină)
bool suntPreaAproape(float x1, float z1, float x2, float z2) {
    float dist = sqrt(pow(x1 - x2, 2) + pow(z1 - z2, 2));
    return dist < 3.0f; // Raza de coliziune
}

//P3_b
float lightPos[] = { 30.0f, 50.0f, 30.0f, 1.0f }; 







// P2: DECLARATII TEXTURI
GLuint texIarba, texOrizont, texAsfalt, texCladire, texCladire2, texCladire3, texApa;
GLuint texCopac, texFrunze;
// Funcția de încărcare a texturilor
GLuint LoadTextureReal(const char* filename) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_set_flip_vertically_on_load(true);
    int w, h, nrChannels;
    unsigned char* data = stbi_load(filename, &w, &h, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);
    return texture;
}
// Funcția matematică pentru relief
float calculInaltime(float x, float z) {
    
    float y = 0.3f * sin(x * 0.2f) + 0.3f * cos(z * 0.2f);

    // Muntele 1
    float dist1 = sqrt((x - (-8.0f)) * (x - (-8.0f)) + (z - 0.0f) * (z - 0.0f));
    if (dist1 < 2.5f) y += 5.0f * cos((dist1 / 2.5f) * 1.57079f);

    // Muntele 2
    float dist2 = sqrt((x - 2.0f) * (x - 2.0f) + (z - (-2.0f)) * (z - (-2.0f)));
    if (dist2 < 3.0f) y += 7.0f * cos((dist2 / 3.0f) * 1.57079f);

    // Muntele 3
    float dist3 = sqrt((x - 10.0f) * (x - 10.0f) + (z - 2.0f) * (z - 2.0f));
    if (dist3 < 2.0f) y += 4.0f * cos((dist3 / 2.0f) * 1.57079f);

    return y;
}


// P3_b: FUNCTII NOI PENTRU UMBRA PE PLAN INCLINAT
void getNormala(float x, float z, float normal[3]) {
    float pas = 0.1f;
    float hL = calculInaltime(x - pas, z);
    float hR = calculInaltime(x + pas, z);
    float hD = calculInaltime(x, z - pas);
    float hU = calculInaltime(x, z + pas);
    normal[0] = hL - hR;
    normal[1] = 2.0f * pas;
    normal[2] = hD - hU;
    float lungime = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
    normal[0] /= lungime; normal[1] /= lungime; normal[2] /= lungime;
}
void aplicaMatriceUmbraInclinata(float punctSol[3], float normala[3], float lumina[4], float lungimeUmbra) {
    // P3_b: LUNGIME UMBRA
    // Factor de scalare: 1.0 = normal, 2.0 = dublu, 0.5 = jumatate
   

    // Scalează componentele orizontale ale luminii înainte de calcul
    float luminaScalata[4];
    luminaScalata[0] = lumina[0] * lungimeUmbra; // Întinde pe X
    luminaScalata[1] = lumina[1];               // Înălțimea rămâne aceeași
    luminaScalata[2] = lumina[2] * lungimeUmbra; // Întinde pe Z
    luminaScalata[3] = lumina[3];
  

    float d = -(normala[0] * punctSol[0] + normala[1] * punctSol[1] + normala[2] * punctSol[2]);
    float dot = normala[0] * luminaScalata[0] + normala[1] * luminaScalata[1] + normala[2] * luminaScalata[2] + d * luminaScalata[3];
    float shadowMat[16];

    // Matricea standard (S9), folosind lumina scalată
    shadowMat[0] = dot - luminaScalata[0] * normala[0]; shadowMat[4] = -luminaScalata[0] * normala[1]; shadowMat[8] = -luminaScalata[0] * normala[2]; shadowMat[12] = -luminaScalata[0] * d;
    shadowMat[1] = -luminaScalata[1] * normala[0]; shadowMat[5] = dot - luminaScalata[1] * normala[1]; shadowMat[9] = -luminaScalata[1] * normala[2]; shadowMat[13] = -luminaScalata[1] * d;
    shadowMat[2] = -luminaScalata[2] * normala[0]; shadowMat[6] = -luminaScalata[2] * normala[1]; shadowMat[10] = dot - luminaScalata[2] * normala[2]; shadowMat[14] = -luminaScalata[2] * d;
    shadowMat[3] = -luminaScalata[3] * normala[0]; shadowMat[7] = -luminaScalata[3] * normala[1]; shadowMat[11] = -luminaScalata[3] * normala[2]; shadowMat[15] = dot - luminaScalata[3] * d;

    glMultMatrixf(shadowMat);
}






void drawSkybox() {
    float d = 70.0f; float yJos = -15.0f;
    glBindTexture(GL_TEXTURE_2D, texOrizont);
    // SEARA: Colorăm ușor cerul spre o nuanță de amurg (portocaliu/violet)
    glColor3f(0.8f, 0.6f, 0.7f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, yJos, -d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(d, yJos, -d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, -d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-d, yJos, d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, yJos, -d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, -d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-d, d, d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, yJos, -d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(d, yJos, d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(d, d, d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, -d);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(d, yJos, d);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-d, yJos, d);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-d, d, d);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(d, d, d);
    glEnd();
}
void drawRelief() {
    glBindTexture(GL_TEXTURE_2D, texIarba);
    float dimensiune = 50.0f; float pas = 0.5f;
    glBegin(GL_QUADS);
    for (float z = -dimensiune; z < dimensiune; z += pas) {
        for (float x = -dimensiune; x < dimensiune; x += pas) {
            float y1 = calculInaltime(x, z);
            float y2 = calculInaltime(x + pas, z);
            float y3 = calculInaltime(x + pas, z + pas);
            float y4 = calculInaltime(x, z + pas);
            glTexCoord2f((x + dimensiune) / 5.0f, (z + dimensiune) / 5.0f); glVertex3f(x, y1, z);
            glTexCoord2f((x + pas + dimensiune) / 5.0f, (z + dimensiune) / 5.0f); glVertex3f(x + pas, y2, z);
            glTexCoord2f((x + pas + dimensiune) / 5.0f, (z + pas + dimensiune) / 5.0f); glVertex3f(x + pas, y3, z + pas);
            glTexCoord2f((x + dimensiune) / 5.0f, (z + pas + dimensiune) / 5.0f); glVertex3f(x, y4, z + pas);
        }
    }
    glEnd();
}
void drawCircuit() {
    glBindTexture(GL_TEXTURE_2D, texAsfalt);
    float latimeDrum = 1.5f; float pas = 0.5f; float centruX = 2.0f;
    glBegin(GL_QUADS);
    for (float z = 14.0f; z < 45.0f; z += pas) {
        float v = z / 3.0f; float vN = (z + pas) / 3.0f;
        glTexCoord2f(v, 0.0f);  glVertex3f(centruX - latimeDrum, calculInaltime(centruX - latimeDrum, z) + 0.15f, z);
        glTexCoord2f(v, 1.0f);  glVertex3f(centruX + latimeDrum, calculInaltime(centruX + latimeDrum, z) + 0.15f, z);
        glTexCoord2f(vN, 1.0f); glVertex3f(centruX + latimeDrum, calculInaltime(centruX + latimeDrum, z + pas) + 0.15f, z + pas);
        glTexCoord2f(vN, 0.0f); glVertex3f(centruX - latimeDrum, calculInaltime(centruX - latimeDrum, z + pas) + 0.15f, z + pas);
    }
    for (float z = -45.0f; z < -18.0f; z += pas) {
        float v = z / 3.0f; float vN = (z + pas) / 3.0f;
        glTexCoord2f(v, 0.0f);  glVertex3f(centruX - latimeDrum, calculInaltime(centruX - latimeDrum, z) + 0.15f, z);
        glTexCoord2f(v, 1.0f);  glVertex3f(centruX + latimeDrum, calculInaltime(centruX + latimeDrum, z) + 0.15f, z);
        glTexCoord2f(vN, 1.0f); glVertex3f(centruX + latimeDrum, calculInaltime(centruX + latimeDrum, z + pas) + 0.15f, z + pas);
        glTexCoord2f(vN, 0.0f); glVertex3f(centruX - latimeDrum, calculInaltime(centruX - latimeDrum, z + pas) + 0.15f, z + pas);
    }
    glEnd();
    float cCX = 2.0f, cCZ = -2.0f, rC = 16.0f; int seg = 60;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; i++) {
        float th = 2.0f * 3.14159f * (float)i / (float)seg;
        float xM = cCX + (rC + latimeDrum) * cos(th); float zM = cCZ + (rC + latimeDrum) * sin(th);
        float xm = cCX + (rC - latimeDrum) * cos(th); float zm = cCZ + (rC - latimeDrum) * sin(th);
        glTexCoord2f((float)i / 3.0f, 1.0f); glVertex3f(xM, calculInaltime(xM, zM) + 0.16f, zM);
        glTexCoord2f((float)i / 3.0f, 0.0f); glVertex3f(xm, calculInaltime(xm, zm) + 0.16f, zm);
    }
    glEnd();
}





//P3_b
void drawObiectStatic(GLuint textura, float L, float H) {
    glBindTexture(GL_TEXTURE_2D, textura);
    glBegin(GL_QUADS);

    // Fata fata (axa Z pozitiva)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 0); glVertex3f(-L, 0, L);
    glTexCoord2f(1, 0); glVertex3f(L, 0, L);
    glTexCoord2f(1, 1); glVertex3f(L, H, L);
    glTexCoord2f(0, 1); glVertex3f(-L, H, L);

    // Fata spate (axa Z negativa)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0, 0); glVertex3f(L, 0, -L);
    glTexCoord2f(1, 0); glVertex3f(-L, 0, -L);
    glTexCoord2f(1, 1); glVertex3f(-L, H, -L);
    glTexCoord2f(0, 1); glVertex3f(L, H, -L);

    // Fata stanga (axa X negativa)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-L, 0, -L);
    glTexCoord2f(1, 0); glVertex3f(-L, 0, L);
    glTexCoord2f(1, 1); glVertex3f(-L, H, L);
    glTexCoord2f(0, 1); glVertex3f(-L, H, -L);

    // Fata dreapta (axa X pozitiva)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(L, 0, L);
    glTexCoord2f(1, 0); glVertex3f(L, 0, -L);
    glTexCoord2f(1, 1); glVertex3f(L, H, -L);
    glTexCoord2f(0, 1); glVertex3f(L, H, L);

    // Capacul sus (axa Y pozitiva)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0, 0); glVertex3f(-L, H, L);
    glTexCoord2f(1, 0); glVertex3f(L, H, L);
    glTexCoord2f(1, 1); glVertex3f(L, H, -L);
    glTexCoord2f(0, 1); glVertex3f(-L, H, -L);

    glEnd();
}

void drawCopac(float rT, float hT) {
    drawObiectStatic(texCopac, rT, hT);
    glBindTexture(GL_TEXTURE_2D, texFrunze);
    glBegin(GL_TRIANGLES);
    float hC = hT * 2.0f; float rC = rT * 4.0f;
    for (int i = 0; i < 4; i++) {
        float u = i * 1.5707f; float uN = (i + 1) * 1.5707f;
        glTexCoord2f(0.5, 1); glVertex3f(0, hT + hC, 0);
        glTexCoord2f(0, 0); glVertex3f(rC * cos(u), hT, rC * sin(u));
        glTexCoord2f(1, 0); glVertex3f(rC * cos(uN), hT, rC * sin(uN));
    }
    glEnd();
}




//P3_b LUMINA
void drawPataLumina(float r, float g, float b, float raza) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Efect de adunare (strălucire)

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(r, g, b, 0.4f); // Centru colorat
    glVertex3f(0.0f, 0.05f, 0.0f);

    glColor4f(r, g, b, 0.0f); // Margine transparentă
    for (int i = 0; i <= 20; i++) {
        float angle = i * 2.0f * 3.14159f / 20.0f;
        glVertex3f(cos(angle) * raza, 0.05f, sin(angle) * raza);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}





// P3_c: Functie modulara pentru desen geometrie stalp
void drawGeometrieStalp() {
    drawObiectStatic(texCladire3, 0.1f, 6.0f); // Trunchi
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0.0f, 6.0f, 0.0f);
        glRotatef(i * 120.0f, 0, 1, 0);
        glTranslatef(0.0f, 0.0f, 0.4f);
        drawObiectStatic(texCladire3, 0.05f, 0.1f); // Brat
        glPopMatrix();
    }
}
// P3_c: Desen stalp cu intensitate variabila a becurilor
void drawStalpComplex(float px, float pz, float r, float g, float b) {
    float hSol = calculInaltime(px, pz);

    // P3_b ADAUGĂM LUMINA LA BAZĂ
    glPushMatrix();
    glTranslatef(px, hSol, pz);
    drawPataLumina(r, g, b, 4.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(px, hSol, pz);
    drawGeometrieStalp();
    // Becuri cu culoare/intensitate custom
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0.0f, 6.0f, 0.0f);
        glRotatef(i * 120.0f, 0, 1, 0);
        glDisable(GL_LIGHTING);
        glColor3f(r, g, b); // P3_c_luminozitate_diferita
        glTranslatef(0.0f, -0.1f, 0.4f);
        glutSolidSphere(0.15f, 10, 10);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }
    glPopMatrix();
}


// **_****_C1_a & C2_b: FUNCȚIE DESENARE MAȘINĂ 
void drawMasinaGenerica(float x, float z, float rot, float r, float g, float b) {
    // Calculăm înălțimea solului în punctul unde se află centrul mașinii
    float h = calculInaltime(x, z);

    // Ridicăm caroseria mașinii la h + 0.7f (buffer ridicare)
    float bufferRidicare = 0.7f;

    glPushMatrix();
    glTranslatef(x, h + bufferRidicare, z);
    glRotatef(rot, 0, 1, 0);

    //1. CORPUL MAȘINII (Șasiul)
    glColor3f(r, g, b);
    glPushMatrix();
    glScalef(1.2f, 0.5f, 2.0f);
    drawObiectStatic(0, 0.5f, 1.0f);
    glPopMatrix();

    //2. CABINA
    glColor3f(0.2f, 0.2f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, -0.2f);
    glScalef(0.8f, 0.5f, 1.0f);
    drawObiectStatic(0, 0.5f, 1.0f);
    glPopMatrix();

    //3. ROȚILE (CORECTATE: ROTUNDE PE LATERAL ȘI PE DRUM)
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.1f, 0.1f, 0.1f); // Negru cauciuc

    // Poziționare roți la caroserie
    float pozRotiX[4] = { 0.65f, -0.65f, 0.65f, -0.65f };
    float pozRotiZ[4] = { 0.75f, 0.75f, -0.75f, -0.75f };

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        // Coborâm centrul roții cu 0.4f față de caroseria ridicată
        glTranslatef(pozRotiX[i], -0.4f, pozRotiZ[i]);

        // ROTIRE: Întoarcem polii sferei cu 90 de grade pe axa Y
        // Astfel, cercul se vede din lateralul mașinii
        glRotatef(90.0f, 0, 1, 0);

        // SCALARE: O turtim pe axa Z locală (lățimea roții)
        // Valorile 1.0 la X și Y asigură că rămâne un CERC PERFECT
        glScalef(1.0f, 1.0f, 0.3f);

        // Desenăm discul folosind o sferă turtită. 
        // Raza 0.3f + coborârea 0.4f din centrul aflat la h+0.7f pune baza la fix h.
        glutSolidSphere(0.3f, 20, 20);
        glPopMatrix();
    }

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}





// ****_C2_a: DESENARE PASĂRE
void drawPasare(int i) {
    glPushMatrix();
    glTranslatef(birdX[i], birdY[i], birdZ[i]);
    glRotatef(birdAngle[i], 0, 1, 0); // Pasărea "privește" unde zboară

    // Corpul
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glScalef(0.4f, 0.2f, 0.6f);
    glutSolidSphere(1.0, 8, 8);
    glPopMatrix();

    // Aripi
    glColor3f(0.1f, 0.1f, 0.1f);
    for (int j = -1; j <= 1; j += 2) {
        glPushMatrix();
        glRotatef(wingAngle * j, 0, 0, 1);
        glTranslatef(j * 0.4f, 0, 0);
        glScalef(0.6f, 0.05f, 0.3f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
    glPopMatrix();
}

// ****_C2_a: LOGICĂ MIȘCARE COMPLET ALEATORIE ("AIUREA")
void updatePasari() {
    float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    wingAngle = sin(t * 15.0f) * 40.0f; // Bătaie rapidă din aripi

    for (int i = 0; i < 3; i++) {
        // 1. Modificăm ușor direcția (haotic)
        // Folosim sin cu frecvență mare pentru a simula mici devieri de la traseu
        birdDir[i] += sin(t * (2.0f + i)) * 1.5f;

        // 2. Calculăm vectorul de deplasare (Unit Circle)
        float rad = birdDir[i] * 3.14159f / 180.0f;
        float vx = sin(rad) * 0.1f; // 0.1f este viteza de zbor
        float vz = cos(rad) * 0.1f;

        // 3. Actualizăm poziția (se adună, deci nu revine la zero)
        birdX[i] += vx;
        birdZ[i] += vz;

        // Variație mică pe înălțime
        birdY[i] += cos(t * 1.5f + i) * 0.02f;

        // 4. Orientăm modelul păsării spre direcția de mers
        birdAngle[i] = birdDir[i];

        // 5. Limitare (Dacă ies prea mult din scenă, le rotim inapoi)
        if (birdX[i] > 60.0f) birdX[i] = -60.0f;
        if (birdX[i] < -60.0f) birdX[i] = 60.0f;
        if (birdZ[i] > 60.0f) birdZ[i] = -60.0f;
        if (birdZ[i] < -60.0f) birdZ[i] = 60.0f;
    }
}




void drawFantana(float r, float h) {
    drawObiectStatic(texCladire, r, h * 0.3f);
    glBindTexture(GL_TEXTURE_2D, texApa);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-r, h * 0.31f, r); glTexCoord2f(1, 0); glVertex3f(r, h * 0.31f, r);
    glTexCoord2f(1, 1); glVertex3f(r, h * 0.31f, -r); glTexCoord2f(0, 1); glVertex3f(-r, h * 0.31f, -r);
    glEnd();
}




void drawOras() {
    float cCX = 2.0f, cCZ = -2.0f, rO = 22.0f;
    for (int i = 0; i < 6; i++) {
        float u = (2.0f * 3.14159f * i) / 6.0f;
        float px = cCX + rO * cos(u); float pz = cCZ + rO * sin(u);
        glPushMatrix();
        glTranslatef(px, calculInaltime(px, pz) + 0.01f, pz);
        if (i % 3 == 0) drawObiectStatic(texCladire, 2.0f, 7.0f);
        else if (i % 3 == 1) drawObiectStatic(texCladire2, 1.5f, 4.0f);
        else drawFantana(1.2f, 1.5f);
        glPopMatrix();
    }
    float xL[4] = { -4.5f, 8.5f, -4.5f, 8.5f }; float zL[4] = { 30.0f, 30.0f, -35.0f, -35.0f };
    for (int i = 0; i < 4; i++) {
        glPushMatrix(); glTranslatef(xL[i], calculInaltime(xL[i], zL[i]) + 0.01f, zL[i]);
        drawObiectStatic(texCladire3, 1.8f, 3.5f); glPopMatrix();
    }
   
    float xC[4] = { -15.0f, 18.0f, 0.0f, -0.5f }; // Am adăugat -0.5f
    float zC[4] = { 10.0f, -10.0f, -25.0f, 20.0f }; // Am adăugat 20.0f (între 15 și 25)

    for (int i = 0; i < 4; i++) { // Schimbă limita la 4
        glPushMatrix();
        glTranslatef(xC[i], calculInaltime(xC[i], zC[i]), zC[i]);
        drawCopac(0.3f, 1.5f);
        glPopMatrix();
    }



    // P3_c: Adaugare cei 3 stalpi pe marginea drumului
 // Am adaugat parametri de culoare (R, G, B) pentru a reflecta luminozitati diferite
    drawStalpComplex(-0.5f, 25.0f, 0.6f, 0.4f, 0.2f); // Stâlp 1: Lumină slabă, portocalie
    drawStalpComplex(-0.5f, 35.0f, 1.0f, 1.0f, 0.5f); // Stâlp 2: Lumină normală
    drawStalpComplex(4.5f, -30.0f, 1.0f, 1.0f, 0.5f); // Stâlp 3: Lumină normală


    // P3_c_2stalpi - Acum sunt aliniați de-a lungul drumului
    drawStalpComplex(-0.5f, 15.0f, 1.0f, 1.0f, 0.8f); // Stâlp 4: Lumină puternică, gălbuie
    drawStalpComplex(-0.5f, 45.0f, 1.0f, 1.0f, 0.5f); // Stâlp 5: Lumină normală
}



// **_C1_b: Înregistrarea tuturor obiectelor solide în sistemul de coliziuni
void initObstacole() {
    obstacole.clear(); // Curățăm lista pentru a evita duplicatele

    // 1. BLOCURILE ȘI PISCINA din drawOras
    float cCX = 2.0f, cCZ = -2.0f, rO = 22.0f;
    for (int i = 0; i < 6; i++) {
        float u = (2.0f * 3.14159f * i) / 6.0f;
        float px = cCX + rO * cos(u);
        float pz = cCZ + rO * sin(u);

        if (i % 3 == 0)      adaugaObstacol(px, pz, 2.2f); // Blocuri mari
        else if (i % 3 == 1) adaugaObstacol(px, pz, 1.7f); // Blocuri medii
        else                 adaugaObstacol(px, pz, 1.4f); // Piscina
    }

    // 2. CELE 4 CLĂDIRI DECORATIVE (xL, zL)
    float xL[4] = { -4.5f, 8.5f, -4.5f, 8.5f };
    float zL[4] = { 30.0f, 30.0f, -35.0f, -35.0f };
    for (int i = 0; i < 4; i++) {
        adaugaObstacol(xL[i], zL[i], 2.0f);
    }

    // 3. COPACII (xC, zC)
    float xC[4] = { -15.0f, 18.0f, 0.0f, -0.5f };
    float zC[4] = { 10.0f, -10.0f, -25.0f, 20.0f };
    for (int i = 0; i < 4; i++) {
        adaugaObstacol(xC[i], zC[i], 0.5f); // Copacii au rază mai mică
    }

    // 4. STÂLPII DE ILUMINAT
    float xS[5] = { -0.5f, -0.5f, 4.5f, -0.5f, -0.5f };
    float zS[5] = { 25.0f, 35.0f, -30.0f, 15.0f, 45.0f };
    for (int i = 0; i < 5; i++) {
        adaugaObstacol(xS[i], zS[i], 0.3f); // Stâlpii sunt foarte subțiri
    }
}


// ****_C2_b: ACTUALIZARE MIȘCARE MAȘINI AUTOMATE
void updateMasiniAuto() {
    for (int i = 0; i < 2; i++) {
        // Calculăm poziția viitoare a mașinii automate i
        float viitorAngle = autoAngle[i] + autoSpeed;
        float vX = 2.0f + 16.0f * cos(viitorAngle);
        float vZ = -2.0f + 16.0f * sin(viitorAngle);

        // 1. Verificăm coliziunea cu jucătorul
        bool coliziuneJucator = suntPreaAproape(vX, vZ, objX, objZ);

        // 2. Verificăm coliziunea cu cealaltă mașină automată
        int celalalt = (i == 0) ? 1 : 0;
        float cX = 2.0f + 16.0f * cos(autoAngle[celalalt]);
        float cZ = -2.0f + 16.0f * sin(autoAngle[celalalt]);
        bool coliziuneAltaMasina = suntPreaAproape(vX, vZ, cX, cZ);

        // Actualizăm mișcarea doar dacă drumul e liber
        if (!coliziuneJucator && !coliziuneAltaMasina) {
            autoAngle[i] = viitorAngle;
            if (autoAngle[i] > 6.28318f) autoAngle[i] -= 6.28318f;
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // **_C1_a: CAMERA URMEAZĂ OBIECTUL (Legăm coordonatele camerei de objX și objZ) 
    camY = calculInaltime(objX, objZ) + 3.0f;

    // **_C1_a: Controlul unghiului de vizualizare în jurul obiectului 
    glTranslatef(0.0f, 0.0f, -15.0f); // Distanțare (Zoom out) pentru a vedea mașina
    glRotatef(rotX, 1, 0, 0);         // Înclinare verticală (Săgeți SUS/JOS)
    glRotatef(rotY, 0, 1, 0);         // Rotire orizontală (Săgeți STÂNGA/DREAPTA)

    // **_C1_a: Translatăm sistemul de referință la poziția curentă a mașinii 
    glTranslatef(-objX, -camY, -objZ);

    drawSkybox();
    drawRelief();
    drawCircuit();




    // **_C1_a: Randăm mașina jucătorului (Roșie)
    drawMasinaGenerica(objX, objZ, objRot, 1.0f, 0.0f, 0.0f);

    // ****_C2_b: Randăm mașinile automate (Verde și Galben) pe circuit (centru 2, -2)
    for (int i = 0; i < 2; i++) {
        // Calculăm poziția pe cerc: x = centruX + R * cos, z = centruZ + R * sin
        // Atenție: Folosim autoAngle[i] care este în radiani
        float x_auto = 2.0f + 16.0f * cos(autoAngle[i]);
        float z_auto = -2.0f + 16.0f * sin(autoAngle[i]);

        // Calculăm înălțimea drumului EXACT în acest punct
        float h_auto = calculInaltime(x_auto, z_auto);

        // Calculăm rotația pentru a fi tangentă la drum
        // Formula: unghiul în grade calculat din radiani + 90 sau 180 grade 
        // depinzând de orientarea modelului tău
        float rotGrade = -autoAngle[i] * 180.0f / 3.14159f;

        if (i == 0) {
            // Mașina Verde - merge normal
            drawMasinaGenerica(x_auto, z_auto, rotGrade, 0.0f, 1.0f, 0.0f);
        }
        else {
            // Mașina Galbenă - o rotim cu 180 grade dacă vrem să meargă în sens opus 
            // sau o lăsăm așa dacă merge în același sens
            drawMasinaGenerica(x_auto, z_auto, rotGrade, 1.0f, 1.0f, 0.0f);
        }
    }


   

    // ****_C2_a: Randare cele 3 păsări
    for (int i = 0; i < 3; i++) drawPasare(i);



    // P3_b: SECTIUNE UMBRE (Dezactivam iluminarea si texturile pentru proiectie)
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




    // 1. Umbrele Pomi Decorativi (generate de Soare/lightPos)
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f); // Negru usor transparent
    float xC[4] = { -15.0f, 18.0f, 0.0f, -0.5f };
    float zC[4] = { 10.0f, -10.0f, -25.0f, 20.0f };
    for (int i = 0; i < 3; i++) {
        float hSol = calculInaltime(xC[i], zC[i]);
        float n[3]; getNormala(xC[i], zC[i], n);
        float pB[3] = { xC[i], hSol + 0.02f, zC[i] };
        glPushMatrix();
        aplicaMatriceUmbraInclinata(pB, n, lightPos, 2.5f);
        glTranslatef(xC[i], hSol, zC[i]);
        drawCopac(0.3f, 1.5f);
        glPopMatrix();
    }


    // P3_c: Umbra multipla (Copacul central între două surse de lumină/stâlpi)
    // Intensitatea umbrei este data de intensitatea luminii becurilor
    float xCopacM = -0.5f; float zCopacM = 20.0f;
    float hSolM = calculInaltime(xCopacM, zCopacM);
    float nM[3]; getNormala(xCopacM, zCopacM, nM);
    float pBM[3] = { xCopacM, hSolM + 0.03f, zCopacM };

    // Sursa 1: Stalp Z=15 (Lumina 1.0 -> Umbra intensa Alpha 0.6)
    float sursaS1[4] = { -0.5f, hSolM + 6.0f, 15.0f, 1.0f };
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glPushMatrix();
    aplicaMatriceUmbraInclinata(pBM, nM, sursaS1, 1.0f);
    glTranslatef(xCopacM, hSolM, zCopacM); drawCopac(0.3f, 1.5f);
    glPopMatrix();

    // Sursa 2: Stalp Z=25 (Lumina 0.6 -> Umbra slaba Alpha 0.2)
    float sursaS2[4] = { -0.5f, hSolM + 6.0f, 25.0f, 1.0f };
    glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
    
    
    
    glPushMatrix();
    aplicaMatriceUmbraInclinata(pBM, nM, sursaS2, 1.0f);
    glTranslatef(xCopacM, hSolM, zCopacM); drawCopac(0.3f, 1.5f);
    glPopMatrix();




    // P3_b & P3_c: UMBRELE STALPILOR
    float xS[5] = { -0.5f, -0.5f, 4.5f, -0.5f, -0.5f };
    float zS[5] = { 25.0f, 35.0f, -30.0f, 15.0f, 45.0f };
    // Vector de intensitate: corelat cu valorile din drawOras (0.6 pentru primul stalp, 1.0 restul)
    float intensitati[5] = { 0.6f, 1.0f, 1.0f, 1.0f, 1.0f };

    for (int i = 0; i < 5; i++) {
        float hSolS = calculInaltime(xS[i], zS[i]);
        float nS[3]; getNormala(xS[i], zS[i], nS);
        float pBS[3] = { xS[i], hSolS + 0.05f, zS[i] };

        // Setăm opacitatea în funcție de intensitatea becului definită mai sus
        float alphaUmbra = (intensitati[i] < 1.0f) ? 0.2f : 0.6f;
        glColor4f(0.0f, 0.0f, 0.0f, alphaUmbra);

        glPushMatrix();
        aplicaMatriceUmbraInclinata(pBS, nS, lightPos, 0.5f);
        glTranslatef(xS[i], hSolS, zS[i]);
        drawGeometrieStalp();

        // Umbra becurilor
        for (int j = 0; j < 3; j++) {
            glPushMatrix();
            glTranslatef(0.0f, 6.0f, 0.0f);
            glRotatef(j * 120.0f, 0, 1, 0);
            glTranslatef(0.0f, -0.1f, 0.4f);
            glutSolidSphere(0.15f, 10, 10);
            glPopMatrix();
        }
        glPopMatrix();
    }
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    drawOras();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    width = w; height = h;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 300.0);
    glMatrixMode(GL_MODELVIEW);
}



// **_P3_a & C1_a: GESTIONARE INPUT TASTATURA (Control Obiect cu W, A, S, D)
void keyboard(unsigned char key, int x, int y) {
    // **_C1_a: Variabile pentru mișcare obiect (mașină) bazate pe rotația actuală 
    float radObj = objRot * 3.14159f / 180.0f;
    float viitorX = objX;
    float viitorZ = objZ;

    switch (key) {
        //**_C1_a: CONTROL OBIECT (MAȘINĂ) folosind W, A, S, D 
    case 'w':
        viitorX += objSpeed * sin(radObj);
        viitorZ += objSpeed * cos(radObj);
        // Blocăm mișcarea dacă lovim o clădire SAU o mașină automată
        if (!verificaColiziune(viitorX, viitorZ) && !verificaColiziuneMasini(viitorX, viitorZ)) {
            objX = viitorX; objZ = viitorZ;
        }
        break;
    case 's':
        viitorX -= objSpeed * sin(radObj);
        viitorZ -= objSpeed * cos(radObj);
        if (!verificaColiziune(viitorX, viitorZ) && !verificaColiziuneMasini(viitorX, viitorZ)) {
            objX = viitorX; objZ = viitorZ;
        }
        break;

    case 'a': // Rotație stânga: modifică unghiul obiectului în World Space 
        objRot += 5.0f;
        break;

    case 'd': // Rotație dreapta
        objRot -= 5.0f;
        break;

    case 27: // ESC pentru ieșire [cite: 436]
        exit(0);
        break;
    }

    // Reafișarea scenei pentru a actualiza poziția obiectului în viewport 
    glutPostRedisplay();
}

// **_C1_a: GESTIONARE TASTE SPECIALE (Rotații cameră în jurul obiectului)
void specialKeys(int key, int x, int y) {
    // Rotațiile (rotX, rotY) definesc acum unghiul de observație al obiectului 
    if (key == GLUT_KEY_UP)    rotX -= 2.0f; // Coboară privirea spre obiect
    if (key == GLUT_KEY_DOWN)  rotX += 2.0f; // Ridică privirea
    if (key == GLUT_KEY_LEFT)  rotY -= 2.0f; // Rotește camera la stânga în jurul mașinii
    if (key == GLUT_KEY_RIGHT) rotY += 2.0f; // Rotește camera la dreapta în jurul mașinii

    glutPostRedisplay(); // Reafișare pentru actualizarea perspectivei
}




int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Scena cu Stalpi Noi si Umbre");

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    texIarba = LoadTextureReal("iarba.jpg");
    texOrizont = LoadTextureReal("orizont.jpg");
    texAsfalt = LoadTextureReal("asfalt.jpg");
    texCladire = LoadTextureReal("cladire.jpg");
    texCladire2 = LoadTextureReal("cladire2.jpg");
    texCladire3 = LoadTextureReal("cladire3.jpg");
    texApa = LoadTextureReal("apa.jpg");
    texCopac = LoadTextureReal("copac.jpg");
    texFrunze = LoadTextureReal("frunze.jpg");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);


    //****_C2_a,b
    glutIdleFunc([]() {
        updatePasari(); // C2_a: Actualizare poziții păsări în fiecare cadru
        updateMasiniAuto(); // C2_b
        glutPostRedisplay();
        });
                             //  s-aa anulat : glutIdleFunc([]() { glutPostRedisplay(); });
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);







    // P3_b: ACTIVARE SI CONFIGURARE ILUMINARE (S9)
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL); // Permite texturilor si culorilor sa interactioneze cu lumina
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Lumina ambientală: nuanțe de albastru/violet închis
    float ambient[] = { 0.3f, 0.2f, 0.4f, 1.0f };
    // Lumina difuză: portocaliu cald (ca un soare la apus)
    float diffuse[] = { 0.9f, 0.5f, 0.3f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);  
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Culoarea fundalului: albastru închis (bleumarin)
    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);





    // P3_c: ACTIVARE BLENDING PENTRU SUPRAPUNERE UMBRE
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initObstacole(); // Înregistrează clădirile în sistemul de coliziuni
    glutMainLoop();
    return 0;
}