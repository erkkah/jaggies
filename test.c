#include "tigr.h"
#include <stdio.h>
#include <math.h>

#include "tinyvec.h"

void setPixel(void* context, int x, int y, int c) {
    Tigr* bmp = (Tigr*) context;
    tigrPlot(bmp, x, y, c ? tigrRGB(0xff, 0xff, 0xff) : tigrRGB(0x22, 0x22, 0x22));
}

void animate(Tigr* screen, float time) {
    static float pos1 = 0;
    static float pos2 = 0;

    const float speed1 = (2 * M_PI) / 4;
    const float speed2 = (2 * M_PI) / 3;

    pos1 += time * speed1;
    pos1 = fmodf(pos1, 2 * M_PI);

    pos2 -= time * speed2;
    pos2 = fmodf(pos2, 2 * M_PI);

    const int outerEdges = 5;
    const int innerEdges = 4;
    tinyPoint points[outerEdges + 1 + innerEdges + 1];
    tinyPoint* p = points;

    {
        float edgePhase = pos1;
        float edgeInc = (2 * M_PI) / outerEdges;
        for(int i = 0; i < outerEdges; i++, p++){
            float x = cosf(edgePhase);
            float y = sinf(edgePhase);
            p->x = x * 100 + 100;
            p->y = y * 100 + 100;
            edgePhase += edgeInc;
        }
        p->x = -2;
        p->y = -2;
        p++;
    }

    {
        float edgePhase = pos2;
        float edgeInc = (2 * M_PI) / innerEdges;
        for(int i = 0; i < innerEdges; i++, p++){
            float x = cosf(edgePhase);
            float y = sinf(edgePhase);
            p->x = x * 90 + 100;
            p->y = y * 90 + 100;
            edgePhase += edgeInc;
        }
        p->x = -1;
        p->y = -1;
    }

    tinyClear();
    tinyPoly(points);

    tinyPoint bar[] = {
        {5, 95},
        {195, 95},
        {195, 105},
        {5, 105},
        {-1, -1}
    };
    tinyPoly(bar);

    int lineStart = (int)(100.f * sinf(pos2) + 100.f);
    int lineEnd = 199 - lineStart;

    tinyLine(0, lineStart, 199, lineEnd);
}

int main() {
    Tigr *screen = tigrWindow(200, 200, "e-ink", TIGR_FIXED);

    int pause = 0;

    while (!tigrClosed(screen))
    {
        float now = tigrTime();
        if(tigrReadChar(screen) != 0) {
            pause ^= 1;
        }
        tigrClear(screen, tigrRGB(0x00, 0x00, 0x00));
        if(pause == 0) {
            animate(screen, now);
        }
        tinyRender(200, 200, setPixel, screen);
        tigrUpdate(screen);
    }
    tigrFree(screen);
    return 0;
}
