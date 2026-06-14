// HOMEWARD — A Hot-Air Balloon's Journey
// main.cpp — window, timeline clock, scene orchestration, the 'L' label toggle.
//
// FOUNDATION BUILD: window + keyframe clock + L toggle + M1 primitives proven
// with a star field and a midpoint circle. Later systems (M2..M5) plug into the
// same display() and the same g_scene state.
#include <GL/freeglut.h>
#include <cstdlib>
#include <cstdio>
#include "common/scene.h"
#include "m1_primitives/primitives.h"

using namespace hw;

// ---- A few stars for the foundation proof ---------------------------------
struct Star { int x, y; float size; };
static std::vector<Star> g_stars;

static void seedStars() {
    g_stars.clear();
    srand(1234);
    for (int i = 0; i < 320; ++i) {
        Star s;
        s.x = rand() % WIN_W;
        s.y = GROUND_Y + 20 + rand() % (int)(WIN_H - GROUND_Y - 20);
        s.size = 1.0f + (rand() % 100) / 60.0f;
        g_stars.push_back(s);
    }
}

// ---- Simple clock (replaced by the full M5 timeline later) ----------------
static int   g_lastMs = 0;

static void advanceClock() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - g_lastMs) / 1000.0f;
    g_lastMs = now;
    if (g_scene.paused) return;
    g_scene.t += dt;
    if (g_scene.t >= CYCLE) g_scene.t -= CYCLE;
}

// ---- 2D projection: gluOrtho2D, 1 world unit == 1 pixel -------------------
static void set2D() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void display() {
    advanceClock();
    glClearColor(0.04f, 0.04f, 0.10f, 1.0f); // pre-dawn indigo
    glClear(GL_COLOR_BUFFER_BIT);
    set2D();

    // Stars (points).
    for (const auto& s : g_stars) {
        m1::setColor(0.9f, 0.9f, 1.0f, 0.8f);
        m1::pointStar(s.x, s.y, s.size);
    }

    // A midpoint-circle "moon" drifting so we can see the clock tick.
    int cx = 200 + (int)(g_scene.t * 4.0f) % WIN_W;
    m1::setColor(1.0f, 0.95f, 0.8f, 1.0f);
    m1::midpointCircle(cx, 600, 40);

    // A DDA and a Bresenham line, to prove both rasterisers.
    m1::setColor(0.6f, 0.8f, 1.0f, 1.0f);
    m1::ddaLine(100, 300, 400, 480);
    m1::setColor(1.0f, 0.6f, 0.6f, 1.0f);
    m1::bresenhamLine(900, 300, 1180, 470);

    glutSwapBuffers();
}

static void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

static void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'l': case 'L': g_scene.labelsOn = !g_scene.labelsOn; break;
        case ' ': g_scene.paused = !g_scene.paused; break;
        case 27: glutLeaveMainLoop(); break; // Esc
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("HOMEWARD - A Hot-Air Balloon's Journey");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);

    seedStars();
    g_lastMs = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    printf("HOMEWARD foundation running. L=labels  Space=pause  Esc=quit\n");
    glutMainLoop();
    return 0;
}
