#include "tinyvec.h"
#include <math.h>
#include <stdio.h>

/*

Tinyvec - a tiny lib for writing vector graphics to a monochrome destination.
Supports filled polygons and lines.

The scene is prepared by adding polygons and lines, and is then rendered
in one continuous flow, one pixel at a time.

*/

typedef tinyPoint Point;

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

Line* addLinePrimitive(int x1, int y1, int x2, int y2, int owner) {
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
void tinyPoly(Point* points) {
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
            current = addLinePrimitive(p1->x, p1->y, p2->x, p2->y, polyID);
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

        current = addLinePrimitive(p1->x, p1->y, first->x1, first->y1, polyID);
        setHorizontalPeak(current, prev);
        setHorizontalPeak(first, current);

        if(!done) {
            // Skip negative marker, on to the next vertex
            p1 += 2;
            p2 = p1 + 1;
        }
    }
}

void tinyLine(int x1, int y1, int x2, int y2) {
    Point points[5] = {
        {x1, y1},
        {x2, y2},
        {x2, y2 + 1},
        {x1, y1 + 1},
        {-1, -1}
    };
    tinyPoly(points);
}

void tinyClear() {
    polyEnd = 0;
    lineEnd = 0;
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
    if ((x <= hitX) && (x + 1 > hitX)) {
        if(l->y1 < l->y2) {
            return (y >= l->y1) && (y < l->y2);
        } else {
            return (y > l->y2) && (y <= l->y1);
        }
    } else {
        return 0;
    }
}

void tinyRender(int width, int height, pixelSetter setter, void* context) {
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
                    Poly* poly = polys + l->owner;
                    poly->inside ^= 1;
                    inside += poly->inside ? 1 : -1;
                }
            }
            setter(context, x, y, inside);
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
