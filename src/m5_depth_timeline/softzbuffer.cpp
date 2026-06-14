// m5_depth_timeline/softzbuffer.cpp — 3D balloon (depth test) + software Z-buffer.
#include "m5_depth_timeline/softzbuffer.h"
#include "m1_primitives/primitives.h"
#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>

namespace hw { namespace m5 {

static GLUquadric* g_quad = nullptr;

// The balloon descends and weaves among the towers; the hardware depth buffer
// (enabled in m4::drawCity) decides front/behind per pixel.
void drawBalloon3D(const SceneState& s){
    if(!g_quad) g_quad = gluNewQuadric();
    // descent progress within the city segment
    float p = clampf((s.t - SEG_TIMES[4]) / (SEG_TIMES[5]-SEG_TIMES[4]), 0.0f, 1.0f);
    float bx = 10.0f*std::sin(s.t*0.5f);
    float bz = -8.0f - 36.0f*p + 14.0f*std::sin(s.t*0.6f); // sweeps through rows
    float by = 70.0f - 46.0f*p + 1.2f*std::sin(s.t*2.0f);  // sink + bob
    float R  = 6.0f;

    glPushMatrix();
    glTranslatef(bx,by,bz);
    // envelope
    glColor3f(0.90f,0.32f,0.28f);
    gluSphere(g_quad, R, 18, 14);
    // a yellow band
    glColor3f(0.98f,0.85f,0.35f);
    glPushMatrix(); glScalef(1.01f,0.34f,1.01f); gluSphere(g_quad,R,18,8); glPopMatrix();
    // basket
    glColor3f(0.45f,0.30f,0.14f);
    glTranslatef(0,-R-2.0f,0);
    glutSolidCube(2.4f);
    glPopMatrix();
}

// ---------------------------------------------------------------------------
// Software Z-buffer: rasterise two overlapping triangles into our own colour +
// depth arrays, keeping the nearest fragment per pixel. Proof the algorithm is
// ours (the scene above uses OpenGL's hardware depth buffer).
static const int ZW=130, ZH=96;
static std::vector<float> g_zbuf;          // computed once
static std::vector<Color> g_cbuf;
static bool g_zdone=false;

static void rasterTri(float* depth, Color* color,
                      Vec2 a,Vec2 b,Vec2 c, float za,float zb,float zc, Color col){
    int minx=(int)std::floor(std::min({a.x,b.x,c.x})), maxx=(int)std::ceil(std::max({a.x,b.x,c.x}));
    int miny=(int)std::floor(std::min({a.y,b.y,c.y})), maxy=(int)std::ceil(std::max({a.y,b.y,c.y}));
    minx=std::max(minx,0); miny=std::max(miny,0); maxx=std::min(maxx,ZW-1); maxy=std::min(maxy,ZH-1);
    float area=(b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
    if(std::fabs(area)<1e-4f) return;
    for(int y=miny;y<=maxy;++y) for(int x=minx;x<=maxx;++x){
        float px=x+0.5f, py=y+0.5f;
        float w0=((b.x-px)*(c.y-py)-(b.y-py)*(c.x-px))/area;
        float w1=((c.x-px)*(a.y-py)-(c.y-py)*(a.x-px))/area;
        float w2=1.0f-w0-w1;
        if(w0<0||w1<0||w2<0) continue;
        float z=w0*za+w1*zb+w2*zc;
        int idx=y*ZW+x;
        if(z<depth[idx]){ depth[idx]=z; color[idx]=col; }   // <-- the depth test
    }
}

static void buildSoftZ(){
    g_zbuf.assign(ZW*ZH, std::numeric_limits<float>::infinity());
    g_cbuf.assign(ZW*ZH, Color{0.08f,0.08f,0.12f,1});
    // Triangle 1 (red), slanted in depth front-left to back-right.
    rasterTri(g_zbuf.data(), g_cbuf.data(),
              {18,18},{104,30},{40,84}, 0.2f,0.9f,0.5f, {0.90f,0.30f,0.28f,1});
    // Triangle 2 (blue), crossing it the other way in depth.
    rasterTri(g_zbuf.data(), g_cbuf.data(),
              {96,16},{120,80},{30,66}, 0.85f,0.25f,0.6f, {0.35f,0.55f,0.95f,1});
    g_zdone=true;
}

void drawSoftZBufferDemo(){
    if(!g_zdone) buildSoftZ();
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    int ox=WIN_W-ZW*2-16, oy=16, sc=2;
    // panel
    glColor4f(0,0,0,0.5f);
    glBegin(GL_QUADS); glVertex2f(ox-8,oy-8); glVertex2f(ox+ZW*sc+8,oy-8);
                       glVertex2f(ox+ZW*sc+8,oy+ZH*sc+24); glVertex2f(ox-8,oy+ZH*sc+24); glEnd();
    glPointSize((float)sc);
    glBegin(GL_POINTS);
    for(int y=0;y<ZH;++y) for(int x=0;x<ZW;++x){
        const Color& c=g_cbuf[y*ZW+x];
        glColor4f(c.r,c.g,c.b,1);
        glVertex2i(ox+x*sc, oy+y*sc);
    }
    glEnd();
    glPointSize(1.0f);
    glColor4f(1,1,1,0.8f);
    glRasterPos2f(ox-4, oy+ZH*sc+8);
    const char* t="software Z-buffer (our own depth array)";
    for(const char* p=t;*p;++p) glutBitmapCharacter(GLUT_BITMAP_8_BY_13,*p);
}

} } // namespace hw::m5
