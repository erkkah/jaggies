#include "tigr.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "jaggies.h"

TPixel white = {
    0xff, 0xff, 0xff, 0xff
};

TPixel gray = {
    0x11, 0x11, 0x11, 0xff
};

typedef struct JContext {
    TPixel* bmp;
} JContext;

void setPixel(void* context, JAGGIE_COLOR c) {
    JContext* jc = (JContext*) context;
    TPixel color = tigrRGB(255 - c, c, 255);
    *(jc->bmp) = c ? color : gray;
    jc->bmp++;
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
    jaggiePoint points[1 + outerEdges + 1 + innerEdges];
    jaggiePoint* p = points;
    jaggieClear();

    {
        float edgePhase = pos1;
        float edgeInc = (2 * M_PI) / outerEdges;

        p->x = outerEdges;
        p->y = 1;
        p++;

        for(int i = 0; i < outerEdges; i++, p++){
            float x = cosf(edgePhase);
            float y = sinf(edgePhase);
            p->x = x * 90 + 100;
            p->y = y * 90 + 100;
            edgePhase += edgeInc;
        }
    }

    {
        float edgePhase = pos2;
        float edgeInc = (2 * M_PI) / innerEdges;
        p->x = innerEdges;
        p->y = 0;
        p++;

        for(int i = 0; i < innerEdges; i++, p++){
            float x = cosf(edgePhase);
            float y = sinf(edgePhase);
            p->x = x * 95 + 100;
            p->y = y * 95 + 100;
            edgePhase += edgeInc;
        }
    }

    jaggieColor(20);
    jaggiePoly(points);

    jaggiePoint hbar[] = {
        {4, 0},
        {5, 95},
        {195, 95},
        {195, 105},
        {5, 105},
    };
    jaggieColor(200);
    jaggiePoly(hbar);

    jaggiePoint vbar[] = {
        {4, 0},
        {95, 5},
        {105, 5},
        {105, 195},
        {95, 195},
    };
    jaggieColor(222);
    jaggiePoly(vbar);


    int lineStart = (int)(100.f * sinf(pos2) + 100.f);
    int lineEnd = 199 - lineStart;

    jaggieColor(99);
    jaggieLine(10, lineStart, 189, lineEnd);
    
    jaggieColor(1);
    jaggieLine(0, 5, 5, 0);
    jaggieLine(0, 0, 5, 5);

    jaggieLine(7, 0, 7, 16);
    jaggieLine(0, 8, 16, 8);

    jaggieLine(5, 11, 0, 16);
    jaggieLine(5, 16, 0, 11);
}

static void renderImages(const char* prefix) {
    int len = strlen(prefix);
    char* buf = (char*) malloc(len + 10);

    Tigr* bmp = tigrBitmap(200, 200);
    float time = 0;
    float totalTime = 12;
    float step = 1.0/15.0;
    int pic = 0;
    do {
        time += step;

        tigrClear(bmp, tigrRGB(0, 0, 0));
        animate(bmp, step);
        JContext jc = {
            bmp->pix
        };
        jaggieRender(200, 200, 0, setPixel, &jc);
        sprintf(buf, "%s_%0.3d.png", prefix, pic);
        tigrSaveImage(buf, bmp);

        pic++;
    } while (time < totalTime);

    free(buf);
    tigrFree(bmp);
}

static void renderScreen() {
    Tigr *screen = tigrWindow(200, 200, "jaggies", TIGR_FIXED);

    int pause = 0;
    int markers = 0;

    while (!tigrClosed(screen) && !tigrKeyDown(screen, TK_ESCAPE))
    {
        float time = tigrTime();
        int key = tigrReadChar(screen);
        if(key == ' ') {
            pause ^= 1;
        } else if (key == 'm') {
            markers ^= 1;
        }
        tigrClear(screen, tigrRGB(0, 0, 0));
        if(pause == 0) {
            animate(screen, time);
        }

        JContext jc = {
            screen->pix
        };
        jaggieRender(200, 200, 0, setPixel, &jc);

        if(markers) {
            tigrPlot(screen, 0, 0, tigrRGB(0xff, 0, 0));
            tigrPlot(screen, 199, 0, tigrRGB(0xff, 0, 0));
            tigrPlot(screen, 199, 199, tigrRGB(0xff, 0, 0));
            tigrPlot(screen, 0, 199, tigrRGB(0xff, 0, 0));
        }

        tigrUpdate(screen);
    }
    tigrFree(screen);
}

int main(int argc, char** argv) {
    if (argc == 2) {
        const char* savePrefix = argv[1];

        renderImages(savePrefix);
    } else {
        renderScreen();
    }

    return 0;
}

