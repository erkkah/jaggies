#include "tigr.h"
#include <stdio.h>
#include <math.h>

/*

Tinyvec - a tiny lib for writing vector graphics to a bitmap.

* Filled polygon (done)
* Polygon (todo)
* Line segment (todo)

*/

typedef struct Point {
    int x, y;
} Point;

typedef struct Line {
    int x1, y1;
    int x2, y2;
    float k;
    float m;
    int owner;
    int hpeak;
} Line;

typedef struct Poly {
    int inside;
} Poly;

const int MAX_POLYS = 256;
Poly polys[MAX_POLYS];

const int MAX_LINES = 1024;
Line lines[MAX_LINES];

int polyEnd = 0;
int lineEnd = 0;

/*

!!! Must keep lines in cw order and use half
open intervals for lines to make sure scanline
hits at polygon vertices do not generate
double "hits".

And cover the case where top or bottom sharp corners
get hit by a scan line. If not, this leads to fill
lines bleeding to the right.

*/

Line* addLine(int x1, int y1, int x2, int y2, int owner) {
    int lineID = lineEnd++;
    Line* line = lines + lineID;
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
    line->owner = owner;

    if(x2 == x1) {
        // vertical line marker :)
        line->k = 0;
        line->m = 666;
    } else {
        line->k = (float)(y2 - y1) / (float)(x2 - x1);
        line->m = y1 - (line->k * x1);
    }

    line->hpeak = 0;
    return line;
}

void setHorizontalPeak(Line* current, Line* prev) {
    // Assume current(x1, y1) == prev(x2, y2)

    int currentDir = current->y2 - current->y1;
    int prevDir = prev->y1 - prev->y2;
    if( (currentDir > 0 && prevDir > 0) ||
        (currentDir < 0 && prevDir < 0) )
    {
        current->hpeak = 1;
    }
}

// Points in clock-wise order.
// Auto closes between start and end points.
// Separate segments (for cut-outs) with x = -2.
// Terminate list with x = -1.
void addPoly(Point* points) {
    int polyID = polyEnd++;
    Poly* poly = polys + polyID;
    poly->inside = 0;

    Point* p1 = points;
    Point* p2 = p1 + 1;

    int done = 0;
    while(!done) {
        Line* first = 0;
        Line* current = 0;
        Line* prev = 0;

        while(p2->x >= 0) {
            current = addLine(p1->x, p1->y, p2->x, p2->y, polyID);
            if(first == 0){
                first = current;
            } else {
                // prev is always != 0 here
                setHorizontalPeak(current, prev);
            }
            prev = current;
            p1 = p2;
            p2++;
        }

        if(p2->x == -1) {
            done = 1;
        }

        current = addLine(p1->x, p1->y, first->x1, first->y1, polyID);
        setHorizontalPeak(current, prev);
        setHorizontalPeak(first, current);

        if(!done) {
            // Skip negative marker, on to the next vertex
            p1 += 2;
            p2 = p1 + 1;
        }
    }
}

void clear() {
    polyEnd = 0;
    lineEnd = 0;
}

void addTestPolys() {
/*

0
|
+------------------ 0
|-*-------------
|---------------
|---------------
|---------------
|---------*-----
|---------------
|---------------
|---------------
|---------------
*---------------
*/
    Point p1[] = {
        {2, 1},
        {10, 5},
        {0, 10},
        {-1, -1}
    };

    //addPoly(p1);

    Point p2[] = {
        {26, 10},
        {100, 50},
        {0, 100},
        {-1, -1}
    };

    addPoly(p2);

    Point p3[] = {
        {0, 0},
        {199, 100},
        {100, 199},
        {50, 100},
        {-1, -1}
    };

    addPoly(p3);

    Point p4[] = {
        {175, 175},
        {195, 175},
        {195, 195},
        {175, 195},
        {-1, -1}
    };

    addPoly(p4);

}

int doesPixelCrossLine(int x, int y, Line* l) {

    // Special case covers half open interval start
    // and horizontal peaks.
    if(x == l->x1 && y == l->y1) {
        return  !l->hpeak;
    }

    // Special case for horizontal line
    if(l->y1 == l->y2) {
        return 0;
    }

    int hitX = -1;

    // Special case for vertical line
    if(l->x1 == l->x2) {
        hitX = l->x1;
    } else {
        hitX = round((y - l->m) / l->k);
    }
    //printf("(%d, %d) ? %f, %f, %f, @%ld\n", x, y, k, m, hitX, l - lines);
    if ((x <= hitX) && (x + 1 > hitX)) {
        //printf("\t(%d, %d)\n", l->y1, l->y2);
        if(l->y1 < l->y2) {
            return (y >= l->y1) && (y < l->y2);
        } else {
            return (y > l->y2) && (y <= l->y1);
        }
    } else {
        return 0;
    }
}

void renderPolys(Tigr* bmp) {
    int height = 200;
    int width = 200;

    // One scanline at a time
    for(int y = 0; y < height; y++){
        // Clear "inside" state
        for(Poly* p = polys; p < polys + polyEnd; p++) {
            p->inside = 0;
        }
        int inside = 0;
        for(int x = 0; x < width; x++) {
            for(Line* l = lines; l < lines + lineEnd; l++) {
                if(doesPixelCrossLine(x, y, l)) {
                    //printf("Cross at (%d, %d, %ld)\n", x, y, l - lines);
                    Poly* poly = polys + l->owner;
                    poly->inside ^= 1;
                    inside += poly->inside ? 1 : -1;
                }
            }
            if(inside){
                tigrPlot(bmp, x, y, tigrRGB(0xff, 0xff, 0xff));
            } else {
                tigrPlot(bmp, x, y, tigrRGB(0x22, 0x22, 0x22));
            }
        }
    }
}

void printLines() {
    for(Line* l = lines; l < lines + lineEnd; l++) {
        printf("l(%d, %d)-(%d, %d) k=%f, m=%f, peak=%d\n",
            l->x1, l->y1,
            l->x2, l->y2,
            l->k, l->m, l->hpeak
        );
    }
}

void animate(Tigr* screen, float time) {
    static float pos = 0;
    const float speed = (2 * M_PI) / 4;
    pos += time * speed;
    pos = fmodf(pos, 2 * M_PI);

    const int numEdges = 5;
    Point points[numEdges + 1 + 4 + 1];

    float edgePhase = pos;
    float edgeInc = (2 * M_PI) / numEdges;
    for(int i = 0; i < numEdges; i++){
        float x = cosf(edgePhase);
        float y = sinf(edgePhase);
        points[i].x = x * 100 + 100;
        points[i].y = y * 100 + 100;
        edgePhase += edgeInc;
    }
    points[numEdges].x = -2;
    points[numEdges].y = -2;

    Point square[] = {
        {80, 80},
        {120, 80},
        {120, 120},
        {80, 120},
        {-1, -1},
        {-9, -9}
    };

    for(Point *s = square, *p = points + numEdges + 1; s->x != -9; s++, p++) {
        *p = *s;
    }

    clear();
    addPoly(points);

    Point bar[] = {
        {5, 95},
        {195, 95},
        {195, 105},
        {5, 105},
        {-1, -1}
    };
    addPoly(bar);

    renderPolys(screen);
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
        } else {
            renderPolys(screen);
        }
        tigrUpdate(screen);
    }
    tigrFree(screen);
    return 0;
}
