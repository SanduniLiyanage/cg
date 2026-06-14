// m4_city_3d/city.cpp — The Architect: the night city in real 3D.
// Far away it is a flat skyline (parallel/orthographic projection); as the
// balloon descends the projection morphs into perspective (a vanishing point),
// the camera orbits (3D rotation) and the city rises + grows into view (3D
// translation + scaling). Depth test + back-face culling (M5) keep it solid.
#include "m4_city_3d/city.h"
#include "m1_primitives/primitives.h"
#include "m5_depth_timeline/softzbuffer.h"
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

namespace hw { namespace m4 {

struct Building { float x, z, w, d, h; int seed; };
static std::vector<Building> g_blocks;

void initCity() {
    g_blocks.clear();
    srand(99);
    for (int gx = -3; gx <= 3; ++gx)
        for (int gz = 0; gz <= 7; ++gz) {
            // leave the central avenue a little emptier
            if (gx == 0 && gz < 2) continue;
            Building b;
            b.x = gx * 15.0f + ((rand()%100)/100.0f-0.5f)*4.0f;
            b.z = -gz * 16.0f - 4.0f;
            b.w = 7.0f + (rand()%40)/10.0f;
            b.d = 7.0f + (rand()%40)/10.0f;
            b.h = 12.0f + (rand()%340)/10.0f - gz*0.6f;
            if (b.h < 8) b.h = 8;
            b.seed = rand();
            g_blocks.push_back(b);
        }
}

// --- Projection matrices (hand-built, column-major for glLoadMatrixf) -------
static void perspM(float* m,float fovyDeg,float asp,float n,float f){
    for(int i=0;i<16;++i) m[i]=0;
    float fv=1.0f/std::tan(deg2rad(fovyDeg)*0.5f);
    m[0]=fv/asp; m[5]=fv; m[10]=(f+n)/(n-f); m[11]=-1; m[14]=2*f*n/(n-f);
}
static void orthoM(float* m,float l,float r,float b,float t,float n,float f){
    for(int i=0;i<16;++i) m[i]=0;
    m[0]=2/(r-l); m[5]=2/(t-b); m[10]=-2/(f-n);
    m[12]=-(r+l)/(r-l); m[13]=-(t+b)/(t-b); m[14]=-(f+n)/(f-n); m[15]=1;
}

// --- Night sky / stars / moon (2D pass) ------------------------------------
static void drawNightSky(const SceneState& s){
    glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glBegin(GL_QUADS);
    glColor3f(s.skyBottom.r,s.skyBottom.g,s.skyBottom.b); glVertex2f(0,0); glVertex2f(WIN_W,0);
    glColor3f(s.skyTop.r,s.skyTop.g,s.skyTop.b);          glVertex2f(WIN_W,WIN_H); glVertex2f(0,WIN_H);
    glEnd();
    // stars (M1 points)
    for(int i=0;i<240;++i){
        float x=(float)((i*7919)%WIN_W);
        float y=260+(float)((i*104729)%460);
        float tw=0.6f+0.4f*std::sin(s.t*2.0f+i);
        m1::setColor(0.92f,0.93f,1.0f,s.starAlpha*tw);
        m1::pointStar((int)x,(int)y,1.0f+((i*13)%100)/80.0f);
    }
    // moon (M1 midpoint circle + glow)
    if(s.moonAlpha>0.01f){
        int mx=980,my=600,r=34;
        glColor4f(0.85f,0.88f,1.0f,0.10f*s.moonAlpha);
        glBegin(GL_TRIANGLE_FAN); glVertex2f(mx,my);
        for(int i=0;i<=26;++i){float a=i/26.0f*2*PI; glVertex2f(mx+r*2.0f*std::cos(a),my+r*2.0f*std::sin(a));}
        glEnd();
        glColor4f(0.95f,0.96f,0.88f,s.moonAlpha);   // solid disc body
        glBegin(GL_TRIANGLE_FAN); glVertex2f(mx,my);
        for(int i=0;i<=40;++i){float a=i/40.0f*2*PI; glVertex2f(mx+r*std::cos(a),my+r*std::sin(a));}
        glEnd();
        m1::setColor(1,1,0.95f,s.moonAlpha); m1::midpointCircle(mx,my,r); // M1 outline
    }
    glDepthMask(GL_TRUE);
}

// --- One building: box (cull-friendly CCW faces) + lit windows -------------
static void drawBuilding(const Building& b, float t){
    float x0=-b.w*0.5f, x1=b.w*0.5f, z0=-b.d*0.5f, z1=b.d*0.5f, y0=0, y1=b.h;
    Color base{0.10f,0.12f,0.20f,1}, top{0.16f,0.18f,0.28f,1};
    glPushMatrix();
    glTranslatef(b.x,0,b.z);
    glBegin(GL_QUADS);
    // +Z (front)  CCW seen from +Z
    glColor3f(base.r,base.g,base.b);
    glVertex3f(x0,y0,z1); glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
    // -Z (back)
    glColor3f(base.r*0.8f,base.g*0.8f,base.b*0.85f);
    glVertex3f(x1,y0,z0); glVertex3f(x0,y0,z0); glVertex3f(x0,y1,z0); glVertex3f(x1,y1,z0);
    // +X (right)
    glColor3f(base.r*0.9f,base.g*0.9f,base.b*0.95f);
    glVertex3f(x1,y0,z1); glVertex3f(x1,y0,z0); glVertex3f(x1,y1,z0); glVertex3f(x1,y1,z1);
    // -X (left)
    glColor3f(base.r*0.7f,base.g*0.7f,base.b*0.8f);
    glVertex3f(x0,y0,z0); glVertex3f(x0,y0,z1); glVertex3f(x0,y1,z1); glVertex3f(x0,y1,z0);
    // top
    glColor3f(top.r,top.g,top.b);
    glVertex3f(x0,y1,z0); glVertex3f(x0,y1,z1); glVertex3f(x1,y1,z1); glVertex3f(x1,y1,z0);
    glEnd();
    // windows on the +Z and +X faces
    int cols=std::max(1,(int)(b.w/2.2f)), rows=std::max(1,(int)(b.h/3.2f));
    glBegin(GL_QUADS);
    for(int r=0;r<rows;++r) for(int c=0;c<cols;++c){
        unsigned h=(unsigned)(b.seed + r*73 + c*131);
        bool lit=((h>>3)&7)>2;
        float fl = lit ? (0.7f+0.3f*std::sin(t*3.0f+h)) : 0.0f;
        if(!lit){ glColor3f(0.05f,0.06f,0.10f); } else glColor3f(1.0f*fl,0.82f*fl,0.42f*fl);
        float wx=x0+ (c+0.5f)/cols*b.w; float wy=y0+ (r+0.4f)/rows*b.h;
        float ww=b.w/cols*0.34f, wh=b.h/rows*0.40f;
        // +Z face
        glVertex3f(wx-ww,wy-wh,z1+0.05f); glVertex3f(wx+ww,wy-wh,z1+0.05f);
        glVertex3f(wx+ww,wy+wh,z1+0.05f); glVertex3f(wx-ww,wy+wh,z1+0.05f);
        // +X face
        float wz=z0+(c+0.5f)/cols*b.d;
        glVertex3f(x1+0.05f,wy-wh,wz-ww); glVertex3f(x1+0.05f,wy-wh,wz+ww);
        glVertex3f(x1+0.05f,wy+wh,wz+ww); glVertex3f(x1+0.05f,wy+wh,wz-ww);
    }
    glEnd();
    glPopMatrix();
}

// Lanterns (M1 midpoint circles) strung over the square at touchdown.
static void drawLanterns(const SceneState& s){
    glDisable(GL_DEPTH_TEST); glDepthMask(GL_FALSE);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    float a = clampf((s.t-SEG_TIMES[5])/6.0f,0.0f,1.0f);
    for(int i=0;i<14;++i){
        float x=120+i*82.0f;
        float y=300+40*std::sin(i*0.9f)+6*std::sin(s.t*2.0f+i);
        Color glow{1.0f,0.7f,0.3f,0.5f*a};
        glColor4f(glow.r,glow.g,glow.b,glow.a);
        glBegin(GL_TRIANGLE_FAN); glVertex2f(x,y);
        for(int k=0;k<=18;++k){float ang=k/18.0f*2*PI; glVertex2f(x+22*std::cos(ang),y+22*std::sin(ang));}
        glEnd();
        m1::setColor(1.0f,0.85f,0.45f,a);
        for(int rr=1;rr<=7;++rr) m1::midpointCircle((int)x,(int)y,rr);
    }
    glDepthMask(GL_TRUE);
}

void drawCity(const SceneState& s){
    drawNightSky(s);

    // --- Projection: morph parallel (ortho) -> perspective -----------------
    float asp=(float)WIN_W/WIN_H;
    float P[16],O[16],M[16];
    perspM(P,50.0f,asp,1.0f,800.0f);
    float oh=54.0f; orthoM(O,-oh*asp,oh*asp,-oh*0.55f,oh*1.05f,1.0f,800.0f);
    float u=clampf(s.cityReveal,0.0f,1.0f);
    for(int i=0;i<16;++i) M[i]=lerp(O[i],P[i],u);
    glMatrixMode(GL_PROJECTION); glLoadMatrixf(M);

    // --- Camera: 3D rotation (orbit) + descent (gluLookAt) -----------------
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    float ang=deg2rad(s.camOrbit);
    float R=130.0f;
    float eyeY=14.0f + s.camHeight*120.0f;
    float eyeX=std::sin(ang)*R, eyeZ=std::cos(ang)*R + 40.0f;
    float cy=18.0f + (1.0f-s.camHeight)*4.0f;
    gluLookAt(eyeX,eyeY,eyeZ, 0,cy,-40, 0,1,0);

    // depth test = the Z-buffer (M5); back-face cull = hidden surface removal
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW);

    // --- City group: 3D translation (rise) + 3D scaling (grow) -------------
    glPushMatrix();
    glTranslatef(0,(s.cityRise-1.0f)*45.0f,0);
    glScalef(s.cityScale,s.cityScale,s.cityScale);

    // ground plane + receding street lines (reinforce the vanishing point)
    glDisable(GL_CULL_FACE);
    glColor3f(0.05f,0.05f,0.09f);
    glBegin(GL_QUADS); glVertex3f(-300,0,60);glVertex3f(300,0,60);
                       glVertex3f(300,0,-400);glVertex3f(-300,0,-400); glEnd();
    glColor3f(0.18f,0.18f,0.10f);
    glBegin(GL_LINES);
    for(float x=-60;x<=60;x+=15){ glVertex3f(x,0.1f,40); glVertex3f(x,0.1f,-380); }
    for(float z=40;z>=-380;z-=16){ glVertex3f(-60,0.1f,z); glVertex3f(60,0.1f,z); }
    glEnd();
    glEnable(GL_CULL_FACE);

    for(const auto& b: g_blocks) drawBuilding(b, s.t);
    glPopMatrix();

    // Balloon weaving between towers (M5) lives in softzbuffer module.
    m5::drawBalloon3D(s);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    if(s.segment==5) drawLanterns(s);

    // Optional software Z-buffer proof (HUD corner) when labels are on.
    if(s.labelsOn) m5::drawSoftZBufferDemo();
}

} } // namespace hw::m4
