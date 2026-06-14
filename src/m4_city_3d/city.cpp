// m4_city_3d/city.cpp — STUB (filled in at the M4/M5 build steps).
#include "m4_city_3d/city.h"
#include <GL/freeglut.h>

namespace hw { namespace m4 {

void initCity() {}

// Placeholder so segments 4-5 don't crash before M4 is built.
void drawCity(const SceneState& s) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glClearColor(s.skyTop.r, s.skyTop.g, s.skyTop.b, 1);
}

} } // namespace hw::m4
