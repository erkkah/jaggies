#include "tinyvec.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef tinyPoint Point;

typedef struct Line {
    int x1, y1;
    int x2, y2;

    // Float based interpolation
    float k;
    float m;

    // Int based interpolation
    int x, y;
    int x0, y0;
    int dx, dy;
    int sx;
    int err0, err;

    int owner;
    int hpeak;
} Line;

typedef struct Poly {
    int inside;
} Poly;

const int MAX_POLYS = 256;
static Poly polys[MAX_POLYS];

const int MAX_LINES = 1024;
static Line lines[MAX_LINES];
static Line* sortedLines[MAX_LINES];

static int polyEnd = 0;
static int lineEnd = 0;

static int sorted = 0;

static void printLine(Line* l) {
    printf("l(%d, %d)-(%d, %d) k=%f, m=%f, peak=%d, dx=%d, dy=%d, y0=%d, err=%d\n",
        l->x1, l->y1,
        l->x2, l->y2,
        l->k, l->m, l->hpeak,
        l->dx, l->dy, l->y0, l->err
    );
}

static void printLines() {
    for(Line** l = sortedLines; l < sortedLines + lineEnd; l++) {
        printLine(*l);
    }
}

static int lineCompareY0(const void* a, const void* b) {
    return (*(Line**)a)->y0 - (*(Line**)b)->y0;
}

static void sortLines() {
    if(!sorted) {
        qsort(sortedLines, lineEnd, sizeof(Line*), lineCompareY0);
        sorted = 1;
    }
}


/*

!!! Must keep lines in cw order and use half
open intervals for lines to make sure scanline
hits at polygon vertices do not generate
double "hits".

And cover the case where top or bottom sharp corners
get hit by a scan line. If not, this leads to fill
lines bleeding to the right.

*/

static Line* addLinePrimitive(int x1, int y1, int x2, int y2, int owner) {
    int lineID = lineEnd++;
    Line* line = lines + lineID;
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
    line->owner = owner;

    // Set up float interpolation
    if(x2 == x1) {
        // vertical line marker :)
        line->k = 0;
        line->m = 666;
    } else {
        line->k = (float)(y2 - y1) / (float)(x2 - x1);
        line->m = y1 - (line->k * x1);
    }

    // Set up int interpolation
    if(y1 < y2) {
        line->x0 = x1;
        line->y0 = y1;

        line->dx = x2 - x1;
        line->dy = y2 - y1;
    } else {
        line->x0 = x2;
        line->y0 = y2;

        line->dx = x1 - x2;
        line->dy = y1 - y2;
    }

    if(line->dx >= 0) {
        line->sx = 1;
    } else {
        line->dx = -line->dx;
        line->sx = -1;
    }

    line->dx *= 2;
    line->dy *= 2;
    line->err = (line->dx > line->dy) ? line->dx : -line->dy;

    line->hpeak = 0;

    sortedLines[lineID] = line;
    sorted = 0;

    return line;
}

static void setHorizontalPeak(Line* current, Line* prev) {
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
int tinyPoly(Point* points) {
    if(polyEnd == MAX_POLYS) {
        return 0;
    }

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

    return 1;
}

int tinyLine(int x1, int y1, int x2, int y2) {
    Point points[5] = {
        {x1, y1},
        {x2, y2},
        {x2, y2 + 1},
        {x1, y1 + 1},
        {-1, -1}
    };
    return tinyPoly(points);
}

void tinyClear() {
    polyEnd = 0;
    lineEnd = 0;
}

static int doesPixelCrossLine(int x, int y, Line* l) {

    // Special case covers half open interval start
    // and horizontal peaks.
    if(x == l->x1 && y == l->y1) {
        return  !l->hpeak;
    }

    // Special case for horizontal line
    if(l->y1 == l->y2) {
        return 0;
    }

    // Line in y - range?
    if(l->y1 < l->y2) {
        if(! ((y > l->y1) && (y < l->y2)) ) {
            return 0;
        }
    } else {
        if(! ((y > l->y2) && (y < l->y1)) ) {
            return 0;
        }
    }

    int hitX = -1;

    // Special case for vertical line
    if(l->x1 == l->x2) {
        hitX = l->x1;
    } else {
        if(0) {
            // Float interpolation
            hitX = round((y - l->m) / l->k);
        } else {
            // Int interpolation
            while(l->y < y) {
                int e = l->err;
                if(e > -l->dx) {
                    l->err -= l->dy;
                    l->x += l->sx;
                }
                if(e < l->dy){
                    l->err += l->dx;
                    l->y++;
                }
            }

            hitX = l->x;
        }
    }

    if ((x <= hitX) && (x + 1 > hitX)) {
        return 1;
    } else {
        return 0;
    }
}

void tinyRender(int width, int height, pixelSetter setter, void* context) {
    // Reset line states
    for(Line* l = lines; l < lines + lineEnd; l++) {
        l->x = l->x0;
        l->y = l->y0;
        l->err = l->err0;
    }

    // Sort lines in Y direction
    sortLines();
    Line** lStart = sortedLines;
    Line** lEnd = lStart;
    Line** lLast = lStart + lineEnd;

    // Render one scanline at a time
    for(int y = 0; y < height; y++) {

        if((*lStart)->y0 > y) {
            // No lines yet, just clear scanline
            for(int x = 0; x < width; x++) {
                setter(context, x, y, 0);
            }
            continue;
        }

        while((*lStart)->y0 + (*lStart)->dy < y) {
            lStart++;
        }

        while(lEnd != lLast && (*lEnd)->y0 <= y) {
            lEnd++;
        }

        // Clear "inside" state
        for(Poly* p = polys; p < polys + polyEnd; p++) {
            p->inside = 0;
        }

        int inside = 0;
        for(int x = 0; x < width; x++) {
            for(Line** l = lStart; l < lEnd; l++) {
                if(doesPixelCrossLine(x, y, *l)) {
                    Poly* poly = polys + (*l)->owner;
                    poly->inside ^= 1;
                    inside += poly->inside ? 1 : -1;
                }
            }
            setter(context, x, y, inside ? 1 : 0);
        }
    }
}
