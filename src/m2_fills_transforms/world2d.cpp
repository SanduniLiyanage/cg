// m2_fills_transforms/world2d.cpp — the daytime 2D scene (segments 0-3).
#include "m2_fills_transforms/world2d.h"
#include "m2_fills_transforms/fills.h"
#include "m2_fills_transforms/transforms.h"
#include "m1_primitives/primitives.h"
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

namespace hw { namespace m2 {

// Bands of the lower scene (screen Y).
static const float HORIZON   = GROUND_Y;       // 235
static const float WATER_HI  = GROUND_Y;       // lake top
static const float WATER_LO  = 185.0f;         // lake bottom
static const float GRASS_TOP = 150.0f;         // foreground grass top

// ---- daylight factor (1 = noon, 0 = night) used to tint the ground --------
static float daylight(const SceneState& s) { return clampf(1.0f - s.starAlpha, 0.10f, 1.0f); }

// ---------------------------------------------------------------------------
// Generic helpers
static std::vector<Vec2> ellipse(float cx, float cy, float rx, float ry, int seg=22) {
    std::vector<Vec2> p;
    for (int i = 0; i < seg; ++i) {
        float a = (float)i / seg * 2.0f * PI;
        p.push_back({ cx + rx*std::cos(a), cy + ry*std::sin(a) });
    }
    return p;
}
static void glDisc(float cx, float cy, float r, Color c, float ry=-1) {
    if (ry < 0) ry = r;
    glColor4f(c.r,c.g,c.b,c.a);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx,cy);
    for (int i=0;i<=26;++i){ float a=i/26.0f*2*PI; glVertex2f(cx+r*std::cos(a), cy+ry*std::sin(a)); }
    glEnd();
}

// ---------------------------------------------------------------------------
// Sky gradient (the gradient-quad approach the visual notes recommend).
static void drawSky(const SceneState& s, float yLo, float yHi, float xL, float xR) {
    glBegin(GL_QUADS);
    glColor4f(s.skyBottom.r,s.skyBottom.g,s.skyBottom.b,1); glVertex2f(xL,yLo); glVertex2f(xR,yLo);
    glColor4f(s.skyTop.r,   s.skyTop.g,   s.skyTop.b,   1); glVertex2f(xR,yHi); glVertex2f(xL,yHi);
    glEnd();
}

// Stars (M1 points).
struct Star { float x,y,sz,ph; };
static std::vector<Star> g_stars;
static void seedStars() {
    g_stars.clear(); srand(7);
    for (int i=0;i<260;++i)
        g_stars.push_back({ (float)(rand()%WIN_W), HORIZON+30+(float)(rand()% (int)(WIN_H-HORIZON-40)),
                            1.0f+(rand()%100)/70.0f, (float)(rand()%628)/100.0f });
}
static void drawStars(const SceneState& s) {
    if (s.starAlpha <= 0.01f) return;
    for (const auto& st : g_stars) {
        float tw = 0.6f + 0.4f*std::sin(s.t*2.0f + st.ph);
        m1::setColor(0.92f,0.93f,1.0f, s.starAlpha*tw);
        m1::pointStar((int)(s.camX + st.x), (int)st.y, st.sz); // fixed to camera
    }
}

// Sun: boundary fill inside a midpoint-circle outline, plus a soft glow.
static void drawSun(const SceneState& s) {
    if (s.sunPos.y < HORIZON-30) return;
    int sx = (int)(s.camX + s.sunPos.x), sy = (int)s.sunPos.y, r = (int)s.sunR;
    // glow (alpha-blended GL disc — decorative)
    glDisc(sx, sy, r*2.6f, s.sunGlow);
    glDisc(sx, sy, r*1.7f, {s.sunGlow.r,s.sunGlow.g,s.sunGlow.b,0.18f});
    // boundary fill the disc
    Canvas cv(sx-r-2, sy-r-2, 2*r+5, 2*r+5);
    markCircleBoundary(cv, sx, sy, r);
    boundaryFill4(cv, r+2, r+2);
    drawFilled(cv, s.sunCore);
    m1::setColor(s.sunCore.r*1.1f, s.sunCore.g*1.1f, s.sunCore.b, 1.0f);
    m1::midpointCircle(sx, sy, r);
}

static void drawMoon(const SceneState& s) {
    if (s.moonAlpha <= 0.01f) return;
    int mx=(int)(s.camX+s.moonPos.x), my=(int)s.moonPos.y, r=(int)s.moonR;
    glDisc(mx,my,r*2.2f,{0.85f,0.88f,1.0f,0.12f*s.moonAlpha});
    Canvas cv(mx-r-2,my-r-2,2*r+5,2*r+5);
    markCircleBoundary(cv,mx,my,r); boundaryFill4(cv,r+2,r+2);
    drawFilled(cv,{0.93f,0.94f,0.85f,s.moonAlpha});
    m1::setColor(1,1,0.95f,s.moonAlpha); m1::midpointCircle(mx,my,r);
}

// Clouds drift across (translation), each at its own speed.
static void drawClouds(const SceneState& s) {
    float day = daylight(s);
    Color c{0.95f,0.96f,1.0f, 0.65f*day + 0.1f};
    struct Cl{ float x,y,sc,sp; };
    static Cl cl[5]={{200,560,1.0f,12},{700,620,1.4f,8},{1050,520,0.9f,16},{420,640,1.1f,10},{900,600,1.3f,6}};
    for (auto&c0:cl){
        float x = c0.x + s.camX*0.5f - s.t*c0.sp; // own drift + slow parallax
        x = std::fmod(x, WIN_W+400.0f); if (x<-200) x+=WIN_W+400;
        float px = s.camX + x, py=c0.y, sc=c0.sc;
        glDisc(px,py,38*sc,c,24*sc); glDisc(px+34*sc,py-4,30*sc,c,20*sc);
        glDisc(px-34*sc,py-2,28*sc,c,18*sc); glDisc(px+8*sc,py+12,30*sc,c,20*sc);
    }
}

// ---- Parallax layers ------------------------------------------------------
// Mountains: scan-line filled triangles + Bresenham ridge (far, factor 0.15).
static void drawMountains(const SceneState& s) {
    float f=0.15f, P=560.0f, day=daylight(s);
    Color m{0.42f*day+0.22f, 0.46f*day+0.26f, 0.58f*day+0.30f, 1.0f}; // atmospheric pale
    glPushMatrix(); glTranslatef(s.camX*(1.0f-f),0,0);
    float base = s.camX*f;
    int i0=(int)std::floor((base-P)/P), i1=(int)std::floor((base+WIN_W+P)/P);
    for (int i=i0;i<=i1;++i){
        float cx=i*P + (float)((i*131)%90);
        float h =160.0f + (float)((i*53)%80);
        std::vector<Vec2> tri={{cx-300,HORIZON},{cx,HORIZON+h},{cx+300,HORIZON}};
        scanlineFill(tri,m);
        m1::setColor(m.r*0.8f,m.g*0.8f,m.b*0.85f,1);
        m1::bresenhamLine((int)(cx-300),(int)HORIZON,(int)cx,(int)(HORIZON+h));
        m1::bresenhamLine((int)cx,(int)(HORIZON+h),(int)(cx+300),(int)HORIZON);
    }
    glPopMatrix();
}

// A wavy filled band (GL solid) for hills / forest / grass.
static void drawBand(const SceneState& s,float f,float top,float amp,float P,Color c){
    glPushMatrix(); glTranslatef(s.camX*(1.0f-f),0,0);
    float base=s.camX*f;
    glColor4f(c.r,c.g,c.b,c.a);
    glBegin(GL_QUAD_STRIP);
    for (float x=base-40;x<=base+WIN_W+40;x+=18){
        float y=top+amp*std::sin(x*0.012f + f*9.0f);
        glVertex2f(x,0); glVertex2f(x,y);
    }
    glEnd();
    glPopMatrix();
}

// Triangle trees scattered on the forest band.
static void drawForest(const SceneState& s){
    float f=0.55f, day=daylight(s);
    Color tc{0.10f*day+0.04f,0.30f*day+0.08f,0.12f*day+0.05f,1};
    glPushMatrix(); glTranslatef(s.camX*(1.0f-f),0,0);
    float base=s.camX*f;
    int i0=(int)std::floor((base-60)/70), i1=(int)std::floor((base+WIN_W+60)/70);
    for (int i=i0;i<=i1;++i){
        float x=i*70 + (float)((i*37)%40);
        float h=34+(float)((i*17)%26);
        float gy=120+8*std::sin(x*0.012f+f*9.0f);
        scanlineFill({{x-14,gy},{x,gy+h},{x+14,gy}}, tc);
    }
    glPopMatrix();
}

// ---- Lake: flood-filled water region + reflection (cached once) -----------
static Canvas* g_lake=nullptr;
void initWorld2D(){
    seedStars();
    // The lake water body is a fixed screen-space strip; flood fill it once.
    if(!g_lake){
        int w=WIN_W, h=(int)(WATER_HI-WATER_LO)+2;
        g_lake=new Canvas(0,(int)WATER_LO,w,h);
        // boundary = the strip's top & bottom edges, then flood the interior.
        for(int x=0;x<w;++x){ g_lake->at(x,0)=1; g_lake->at(x,h-1)=1; }
        floodFill4(*g_lake, w/2, h/2);
    }
}

static void drawLake(const SceneState& s){
    if(!g_lake) return;
    float day=daylight(s);
    // Mirror the sky colours into the water, slightly darkened.
    Color w{ s.skyBottom.r*0.55f+0.05f, s.skyBottom.g*0.6f+0.05f, s.skyBottom.b*0.7f+0.1f, 1.0f };
    // draw water base (the flood-filled region), tiled to follow the camera
    glPushMatrix(); glTranslatef(s.camX,0,0);
    drawFilled(*g_lake, w);
    glPopMatrix();

    // Reflection of the balloon, mirrored across the waterline, clipped to the
    // lake strip with a scissor box (window->viewport pixels).
    glEnable(GL_SCISSOR_TEST);
    // scissor is in real framebuffer pixels — scale the lake band to the window
    int scY = (int)(WATER_LO / WIN_H * s.winH);
    int scH = (int)((WATER_HI - WATER_LO) / WIN_H * s.winH);
    glScissor(0, scY, s.winW, scH);
    glPushMatrix();
    // reflect about y = WATER_HI, with a gentle horizontal shimmer
    glTranslatef(2.0f*std::sin(s.t*2.0f),0,0);
    glScalef(1.0f,-1.0f,1.0f); glTranslatef(0,-2.0f*WATER_HI,0);
    drawBalloon(s,true);
    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);

    // shimmer highlights
    m1::setColor(1,1,1,0.06f+0.04f*day);
    for(int i=0;i<60;++i){
        int x=(int)(s.camX)+ (i*53)%WIN_W;
        int y=(int)WATER_LO + (int)( (WATER_HI-WATER_LO) * (0.2f+0.6f*((i*29)%100)/100.0f) );
        m1::bresenhamLine(x,y,x+10,y);
    }
}

// ---- Foreground props: windmill, farmhouse + flags, power lines -----------
static void drawWindmill(const SceneState& s,float worldX){
    float gy=70; float day=daylight(s);
    // tower
    scanlineFill({{worldX-12,gy-30},{worldX+12,gy-30},{worldX+7,gy+70},{worldX-7,gy+70}},
                 {0.55f*day+0.1f,0.5f*day+0.1f,0.45f*day+0.1f,1});
    Vec2 hub{worldX,gy+72};
    // 4 blades, hand-rotated (M2 rotation), scan-line filled
    for(int b=0;b<4;++b){
        float a=s.t*1.6f + b*PI/2.0f;
        std::vector<Vec2> blade={{hub.x+4,hub.y},{hub.x+10,hub.y},{hub.x+6,hub.y+62},{hub.x-2,hub.y+58}};
        std::vector<Vec2> r=rotate(blade,hub,a);
        scanlineFill(r,{0.9f*day+0.05f,0.9f*day+0.05f,0.85f*day+0.05f,1});
    }
    glDisc(hub.x,hub.y,5,{0.3f,0.28f,0.26f,1});
}

static void drawFarmhouse(const SceneState& s,float worldX){
    float gy=66,day=daylight(s);
    scanlineFill({{worldX-40,gy},{worldX+40,gy},{worldX+40,gy+46},{worldX-40,gy+46}},
                 {0.75f*day+0.06f,0.55f*day+0.05f,0.40f*day+0.04f,1});
    scanlineFill({{worldX-48,gy+46},{worldX,gy+82},{worldX+48,gy+46}},
                 {0.55f*day+0.05f,0.20f*day+0.04f,0.16f*day+0.03f,1});
    // lit window at night
    float lit = 1.0f-day;
    glColor4f(1,0.85f,0.4f,0.4f+0.5f*lit);
    glBegin(GL_QUADS); glVertex2f(worldX-14,gy+12);glVertex2f(worldX+14,gy+12);
                       glVertex2f(worldX+14,gy+34);glVertex2f(worldX-14,gy+34); glEnd();
    // pennant flags on a pole: triangles SHEARED by the wind (M2 shear)
    float poleX=worldX+70, poleBase=gy;
    m1::setColor(0.4f,0.35f,0.3f,1); m1::bresenhamLine((int)poleX,(int)poleBase,(int)poleX,(int)(poleBase+90));
    Color fcols[3]={{0.9f,0.3f,0.3f,1},{0.95f,0.8f,0.3f,1},{0.3f,0.6f,0.9f,1}};
    for(int i=0;i<3;++i){
        float fy=poleBase+86-i*26;
        std::vector<Vec2> flag={{poleX,fy},{poleX,fy-16},{poleX+34,fy-8}};
        float k=0.5f+0.5f*std::sin(s.t*3.0f+i); // wind shear varies
        scanlineFill(shearX(flag,fy-8,k*0.6f), fcols[i]);
    }
}

static void drawPowerLines(const SceneState& s){
    float day=daylight(s);
    Color pole{0.35f*day+0.08f,0.30f*day+0.07f,0.28f*day+0.07f,1};
    float gy=72;
    for(float px=400;px<2200;px+=260){
        scanlineFill({{px-4,gy},{px+4,gy},{px+3,gy+120},{px-3,gy+120}},pole);
    }
    // sagging diagonal wires (Bresenham) between pole tops
    m1::setColor(0.15f,0.15f,0.18f,0.9f);
    for(float px=400;px<2200-260;px+=260){
        float x0=px,x1=px+260,yt=gy+116;
        m1::bresenhamLine((int)x0,(int)yt,(int)((x0+x1)/2),(int)(yt-22));
        m1::bresenhamLine((int)((x0+x1)/2),(int)(yt-22),(int)x1,(int)yt);
        m1::bresenhamLine((int)x0,(int)(yt-10),(int)((x0+x1)/2),(int)(yt-30));
        m1::bresenhamLine((int)((x0+x1)/2),(int)(yt-30),(int)x1,(int)(yt-10));
    }
}

// ---- The balloon ----------------------------------------------------------
void drawBalloon(const SceneState& s, bool reflected){
    float bx = s.camX + s.balloonPos.x;
    float by = s.balloonPos.y;
    float R  = 70.0f * clampf(s.balloonInflate,0.04f,1.0f);
    Vec2 pivot{bx, by-R-26}; // basket pivot (bottom)

    glPushMatrix();
    if(!reflected){ // sway = rotation about the basket
        glTranslatef(pivot.x,pivot.y,0); glRotatef(s.balloonSway*57.2958f,0,0,1);
        glTranslatef(-pivot.x,-pivot.y,0);
    }

    // envelope: boundary-filled disc (M2) inside midpoint-circle outline (M1)
    int icx=(int)bx, icy=(int)by, ir=(int)R;
    if(ir>2){
        Canvas cv(icx-ir-2,icy-ir-2,2*ir+5,2*ir+5);
        markCircleBoundary(cv,icx,icy,ir);
        boundaryFill4(cv,ir+2,ir+2);
        drawFilled(cv,{0.92f,0.36f,0.30f,1});
        // gore panels (DDA diagonals) + accent stripes
        m1::setColor(0.99f,0.85f,0.35f,1);
        for(int g=-2;g<=2;++g){
            m1::ddaLine(icx,icy-ir, (int)(icx+g*R*0.42f), (int)(icy-R*0.1f));
        }
        m1::setColor(0.8f,0.2f,0.22f,1); m1::midpointCircle(icx,icy,ir);
        // tapered bottom of the envelope
        scanlineFill({{bx-R*0.5f,by-R*0.7f},{bx+R*0.5f,by-R*0.7f},{bx+10,by-R-22},{bx-10,by-R-22}},
                     {0.85f,0.30f,0.26f,1});
    }
    // ropes: DDA diagonal lines from envelope to basket
    m1::setColor(0.85f,0.82f,0.7f,1);
    m1::ddaLine((int)(bx-R*0.5f),(int)(by-R*0.7f),(int)(bx-14),(int)(by-R-26));
    m1::ddaLine((int)(bx+R*0.5f),(int)(by-R*0.7f),(int)(bx+14),(int)(by-R-26));
    // basket (scan-line filled trapezoid)
    scanlineFill({{bx-15,by-R-26},{bx+15,by-R-26},{bx+11,by-R-50},{bx-11,by-R-50}},
                 {0.5f,0.32f,0.15f,1});
    // flame (flicker)
    if(!reflected){
        glDisc(bx,by-R-20,6*s.flameFlicker,{1.0f,0.7f,0.2f,0.8f},10*s.flameFlicker);
        glDisc(bx,by-R-20,3*s.flameFlicker,{1.0f,0.95f,0.6f,0.9f},6*s.flameFlicker);
    }
    glPopMatrix();
}

// Long leaning shadow of the balloon on the ground (M2 shear).
static void drawBalloonShadow(const SceneState& s){
    float bx=s.camX+s.balloonPos.x;
    float R=70.0f*clampf(s.balloonInflate,0.04f,1.0f);
    float gy=60;
    std::vector<Vec2> sh=ellipse(bx,gy,R*0.7f,16);
    std::vector<Vec2> sheared=shearX(sh,gy,s.shadowShear);
    scanlineFill(sheared,{0.0f,0.0f,0.05f,0.18f});
}

// ---------------------------------------------------------------------------
void drawWorld2D(const SceneState& s){
    drawSky(s, 0, WIN_H, s.camX, s.camX+WIN_W);
    drawStars(s);
    drawSun(s);
    drawMoon(s);
    drawClouds(s);
    drawMountains(s);
    float day=daylight(s);
    drawBand(s,0.30f, GRASS_TOP+30, 14, 500, {0.30f*day+0.06f,0.45f*day+0.08f,0.32f*day+0.07f,1}); // hills
    drawLake(s);
    drawForest(s);
    drawBand(s,1.0f, GRASS_TOP, 10, 360, {0.16f*day+0.04f,0.32f*day+0.05f,0.14f*day+0.03f,1});     // grass
    // foreground props live at absolute world X and scroll through the window
    drawPowerLines(s);
    drawWindmill(s,1700);
    drawFarmhouse(s,1180);
    drawBalloonShadow(s);
    drawBalloon(s,false);
}

} } // namespace hw::m2
