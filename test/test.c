#include "tigr.h"
#include <stdio.h>
#include <math.h>

#include "jaggies.h"

void setPixel(void* context, JAGGIE_INT x, JAGGIE_INT y, char c) {
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
    jaggiePoint points[outerEdges + 1 + innerEdges + 1];
    jaggiePoint* p = points;

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
            p->x = x * 95 + 100;
            p->y = y * 95 + 100;
            edgePhase += edgeInc;
        }
        p->x = -1;
        p->y = -1;
    }

    jaggieClear();
    jaggiePoly(points);

    jaggiePoint bar[] = {
        {5, 95},
        {195, 95},
        {195, 105},
        {5, 105},
        {-1, -1}
    };
    jaggiePoly(bar);

    int lineStart = (int)(100.f * sinf(pos2) + 100.f);
    int lineEnd = 199 - lineStart;

    jaggieLine(10, lineStart, 189, lineEnd);

    jaggieLine(0, 5, 5, 0);
    jaggieLine(0, 0, 5, 5);

    jaggieLine(5, 10, 0, 15);
    jaggieLine(5, 15, 0, 10);
}

int main() {
    Tigr *screen = tigrWindow(200, 200, "jaggies", TIGR_FIXED);

    int pause = 0;
    int markers = 0;

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE))
    {
        float now = tigrTime();
        int key = tigrReadChar(screen);
        if(key == ' ') {
            pause ^= 1;
        } else if (key == 'm') {
            markers ^= 1;
        }
        tigrClear(screen, tigrRGB(0x00, 0x00, 0x00));
        if(pause == 0) {
            animate(screen, now);
        }
        jaggieRender(200, 200, setPixel, screen);

        if(markers) {
            tigrPlot(screen, 0, 0, tigrRGB(0xff, 0, 0));
            tigrPlot(screen, 199, 0, tigrRGB(0xff, 0, 0));
            tigrPlot(screen, 199, 199, tigrRGB(0xff, 0, 0));
            tigrPlot(screen, 0, 199, tigrRGB(0xff, 0, 0));
        }

        tigrUpdate(screen);
    }
    tigrFree(screen);
    return 0;
}
