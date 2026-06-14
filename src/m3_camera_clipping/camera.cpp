// m3_camera_clipping/camera.cpp — The Camera: window->viewport + clipping.
// The visible frame is the camera WINDOW [camX..camX+WIN_W] x [0..WIN_H], shown
// in the screen VIEWPORT (set in main via gluOrtho2D + glViewport). Birds are
// trimmed at the border with Cohen-Sutherland; slanted rain & the shooting star
// with Liang-Barsky. With labels ON the clip window is inset so the trimming is
// plainly visible.
#include "m3_camera_clipping/camera.h"
#include "m1_primitives/primitives.h"
#include <GL/freeglut.h>
#include <cmath>
#include <vector>

namespace hw { namespace m3 {

Vec2 windowToViewport(Vec2 p, float wl, float wr, float wb, float wt,
                      float vx, float vy, float vw, float vh) {
    return { (p.x-wl)/(wr-wl)*vw + vx, (p.y-wb)/(wt-wb)*vh + vy };
}

// --- Cohen-Sutherland ------------------------------------------------------
enum { INSIDE=0, LEFT=1, RIGHT=2, BOTTOM=4, TOP=8 };
static int outcode(float x,float y,float L,float R,float B,float T){
    int c=INSIDE;
    if(x<L)c|=LEFT; else if(x>R)c|=RIGHT;
    if(y<B)c|=BOTTOM; else if(y>T)c|=TOP;
    return c;
}
static bool clipCS(float x0,float y0,float x1,float y1,
                   float L,float R,float B,float T,
                   float& ox0,float& oy0,float& ox1,float& oy1){
    int c0=outcode(x0,y0,L,R,B,T), c1=outcode(x1,y1,L,R,B,T);
    while(true){
        if(!(c0|c1)){ ox0=x0;oy0=y0;ox1=x1;oy1=y1; return true; } // trivially in
        if(c0&c1) return false;                                    // trivially out
        int co=c0?c0:c1; float x=0,y=0;
        if(co&TOP)   { x=x0+(x1-x0)*(T-y0)/(y1-y0); y=T; }
        else if(co&BOTTOM){ x=x0+(x1-x0)*(B-y0)/(y1-y0); y=B; }
        else if(co&RIGHT){ y=y0+(y1-y0)*(R-x0)/(x1-x0); x=R; }
        else { y=y0+(y1-y0)*(L-x0)/(x1-x0); x=L; }
        if(co==c0){ x0=x;y0=y;c0=outcode(x0,y0,L,R,B,T); }
        else      { x1=x;y1=y;c1=outcode(x1,y1,L,R,B,T); }
    }
}

// --- Liang-Barsky ----------------------------------------------------------
static bool clipLB(float x0,float y0,float x1,float y1,
                   float L,float R,float B,float T,
                   float& ox0,float& oy0,float& ox1,float& oy1){
    float dx=x1-x0, dy=y1-y0, t0=0.0f, t1=1.0f;
    float p[4]={-dx,dx,-dy,dy};
    float q[4]={x0-L,R-x0,y0-B,T-y0};
    for(int i=0;i<4;++i){
        if(std::fabs(p[i])<1e-6f){ if(q[i]<0) return false; }
        else{
            float r=q[i]/p[i];
            if(p[i]<0){ if(r>t1) return false; if(r>t0) t0=r; }
            else      { if(r<t0) return false; if(r<t1) t1=r; }
        }
    }
    ox0=x0+t0*dx; oy0=y0+t0*dy; ox1=x0+t1*dx; oy1=y0+t1*dy;
    return true;
}

// Draw helpers — only the clipped portion is plotted (hand rasterised).
static void birdSeg(float ax,float ay,float bx,float by,float L,float R,float B,float T){
    float cx0,cy0,cx1,cy1;
    if(clipCS(ax,ay,bx,by,L,R,B,T,cx0,cy0,cx1,cy1))
        m1::bresenhamLine((int)cx0,(int)cy0,(int)cx1,(int)cy1);
}
static void rainSeg(float ax,float ay,float bx,float by,float L,float R,float B,float T){
    float cx0,cy0,cx1,cy1;
    if(clipLB(ax,ay,bx,by,L,R,B,T,cx0,cy0,cx1,cy1))
        m1::bresenhamLine((int)cx0,(int)cy0,(int)cx1,(int)cy1);
}

void drawClipped(const SceneState& s){
    float margin = s.labelsOn ? 60.0f : 0.0f;
    float L=s.camX+margin, R=s.camX+WIN_W-margin, B=margin, Top=WIN_H-margin;

    // When labels are on, show the clip window so the trimming is obvious.
    if(s.labelsOn){
        m1::setColor(1,1,1,0.25f);
        m1::bresenhamLine((int)L,(int)B,(int)R,(int)B);
        m1::bresenhamLine((int)L,(int)Top,(int)R,(int)Top);
        m1::bresenhamLine((int)L,(int)B,(int)L,(int)Top);
        m1::bresenhamLine((int)R,(int)B,(int)R,(int)Top);
    }

    // --- Birds: a flock crossing the frame, trimmed by Cohen-Sutherland ----
    if(s.starAlpha < 0.6f){ // daytime only
        m1::setColor(0.15f,0.15f,0.18f,0.9f);
        int N=7;
        for(int i=0;i<N;++i){
            float period=WIN_W+260.0f;
            float sx = std::fmod(i*220.0f + s.t*70.0f, period) - 130.0f; // screen-rel
            float y  = 470.0f + 40.0f*std::sin(i*1.7f) + 10.0f*std::sin(s.t*2.0f+i);
            float x  = s.camX + sx;
            float flap = 6.0f + 4.0f*std::sin(s.t*8.0f + i);
            // a simple V: two wings meeting at the body
            birdSeg(x-14, y, x, y+flap, L,R,B,Top);
            birdSeg(x, y+flap, x+14, y, L,R,B,Top);
        }
    }

    // --- Rain: slanted streaks trimmed by Liang-Barsky ---------------------
    if(s.rainAmount > 0.01f){
        m1::setColor(0.7f,0.8f,1.0f, 0.45f*s.rainAmount);
        float slant = 26.0f;          // diagonal fall
        int count = (int)(150 * s.rainAmount);
        for(int i=0;i<count;++i){
            float phase = (i*53)%1000 / 1000.0f;
            float x = s.camX + std::fmod(i*37.0f, (float)WIN_W) - 30.0f;
            float fall = std::fmod(s.t*620.0f + phase*WIN_H, (float)WIN_H);
            float y = WIN_H - fall;
            rainSeg(x, y, x+slant, y-44, L,R,B,Top);
        }
    }

    // --- Shooting star: one bright slanted streak, Liang-Barsky ------------
    if(s.shootingStar >= 0.0f){
        float p = s.shootingStar;
        float x = s.camX - 120 + p*(WIN_W+240);
        float y = WIN_H-60 - p*240;
        m1::setColor(1,1,0.9f,0.9f);
        rainSeg(x, y, x-90, y+34, L,R,B,Top);   // tail
        m1::setColor(1,1,1,0.6f);
        rainSeg(x, y, x-40, y+15, L,R,B,Top);
    }
}

} } // namespace hw::m3
