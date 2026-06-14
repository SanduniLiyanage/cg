// HOMEWARD — A Hot-Air Balloon's Journey
// main.cpp — window, timeline clock, scene orchestration, the 'L' label toggle.
#include <GL/freeglut.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "common/scene.h"
#include "m1_primitives/primitives.h"
#include "m2_fills_transforms/world2d.h"
#include "m3_camera_clipping/camera.h"
#include "m4_city_3d/city.h"
#include "m5_depth_timeline/timeline.h"

using namespace hw;

static int g_lastMs = 0;
static int g_winW = WIN_W, g_winH = WIN_H;   // actual window size

static void reshape(int w, int h) {
    if (w < 1) w = 1; if (h < 1) h = 1;
    g_winW = w; g_winH = h;
    glViewport(0, 0, w, h);
}

static void advanceClock() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - g_lastMs) / 1000.0f;
    g_lastMs = now;
    if (g_scene.paused) return;
    if (dt > 0.1f) dt = 0.1f;            // clamp after a stall
    g_scene.t += dt;
    if (g_scene.t >= CYCLE) g_scene.t -= CYCLE;
}

// 2D projection: gluOrtho2D as a moving WINDOW over the wide world (M3 mapping).
static void set2DWindow() {
    glViewport(0, 0, g_winW, g_winH);        // the viewport = full window
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(g_scene.camX, g_scene.camX + WIN_W, 0, WIN_H); // the window
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

// Screen-space ortho for the HUD / labels.
static void setHUD() {
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

static void text(float x, float y, const char* s, void* font) {
    glRasterPos2f(x, y);
    for (const char* p = s; *p; ++p) glutBitmapCharacter(font, *p);
}

static void drawLabels() {
    if (!g_scene.labelsOn) return;
    glDisable(GL_DEPTH_TEST);
    setHUD();
    // faint backing panel
    glColor4f(0, 0, 0, 0.35f);
    glBegin(GL_QUADS); glVertex2f(0,WIN_H); glVertex2f(360,WIN_H);
                       glVertex2f(360,WIN_H-26-22*(int)g_scene.activeConcepts.size());
                       glVertex2f(0,WIN_H-26-22*(int)g_scene.activeConcepts.size()); glEnd();
    char hdr[96];
    snprintf(hdr,sizeof hdr,"Segment %d   t=%.1fs", g_scene.segment, g_scene.t);
    glColor4f(1,1,1,0.9f);
    text(10, WIN_H-20, hdr, GLUT_BITMAP_9_BY_15);
    glColor4f(0.8f,0.95f,1.0f,0.85f);
    int y = WIN_H-44;
    for (auto& c : g_scene.activeConcepts) { text(16, y, c.c_str(), GLUT_BITMAP_8_BY_13); y-=22; }
    // help line, bottom
    glColor4f(1,1,1,0.5f);
    text(10, 14, "L labels  Space pause  0-5 segments  <- -> scrub  Esc quit", GLUT_BITMAP_8_BY_13);
}

static void display() {
    advanceClock();
    m5::updateTimeline(g_scene, g_scene.t);
    g_scene.winW = g_winW; g_scene.winH = g_winH;

    glClearColor(g_scene.skyTop.r, g_scene.skyTop.g, g_scene.skyTop.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_scene.t < CITY_T) {
        // -------- 2D world (segments 0-3) --------
        glDisable(GL_DEPTH_TEST);
        set2DWindow();
        m2::drawWorld2D(g_scene);
        m3::drawClipped(g_scene);            // birds (Cohen-Sutherland), rain (Liang-Barsky)
    } else {
        // -------- 3D city (segments 4-5) --------
        m4::drawCity(g_scene);
    }

    // 2D->3D hand-off veil
    if (g_scene.city2DFade > 0.001f) {
        glDisable(GL_DEPTH_TEST); setHUD();
        glColor4f(g_scene.skyTop.r, g_scene.skyTop.g, g_scene.skyTop.b, g_scene.city2DFade);
        glBegin(GL_QUADS); glVertex2f(0,0);glVertex2f(WIN_W,0);
                           glVertex2f(WIN_W,WIN_H);glVertex2f(0,WIN_H); glEnd();
    }

    drawLabels();
    glutSwapBuffers();
}

static void timer(int) { glutPostRedisplay(); glutTimerFunc(16, timer, 0); }

static void jumpToSegment(int seg) {
    if (seg < 0) seg = 0; if (seg > 5) seg = 5;
    g_scene.t = SEG_TIMES[seg] + 0.05f;
}

static void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'l': case 'L': g_scene.labelsOn = !g_scene.labelsOn; break;
        case ' ': g_scene.paused = !g_scene.paused; break;
        case '0': case '1': case '2': case '3': case '4': case '5':
            jumpToSegment(key - '0'); break;
        case 27: glutLeaveMainLoop(); break;
    }
}

static void special(int key, int, int) {
    if (key == GLUT_KEY_RIGHT) { g_scene.t += 3.0f; if (g_scene.t>=CYCLE) g_scene.t-=CYCLE; }
    if (key == GLUT_KEY_LEFT)  { g_scene.t -= 3.0f; if (g_scene.t<0) g_scene.t+=CYCLE; }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("HOMEWARD - A Hot-Air Balloon's Journey");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);

    m2::initWorld2D();
    m4::initCity();
    g_lastMs = glutGet(GLUT_ELAPSED_TIME);

    // Optional CLI: <startSeconds> [pause] [labels] for verification screenshots.
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "pause"))  g_scene.paused = true;
        else if (!strcmp(argv[i], "labels")) g_scene.labelsOn = true;
        else g_scene.t = (float)atof(argv[i]);
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutTimerFunc(16, timer, 0);

    printf("HOMEWARD running.  L=labels  Space=pause  0-5=segments  arrows=scrub  Esc=quit\n");
    glutMainLoop();
    return 0;
}
