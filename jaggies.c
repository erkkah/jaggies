#include "jaggies.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef jaggiePoint Point;

typedef struct Line {
    int x1, y1;
    int x2, y2;

    // Line interpolation state
    int x, y;
    int x0, y0;
    int dx, dy;
    int sx;
    int err0, err;

    // Polygon owner, -1 for free lines.
    int owner;
    int hpeak;
} Line;

typedef struct Poly {
    int inside;
} Poly;

#ifndef JAGGIES_MAX_POLYS
#define JAGGIES_MAX_POLYS 128
#endif

#ifndef JAGGIES_MAX_LINES
#define JAGGIES_MAX_LINES 512
#endif

static Poly polys[JAGGIES_MAX_POLYS];
static Line lines[JAGGIES_MAX_LINES];
static Line* sortedLines[JAGGIES_MAX_LINES];

static int polyEnd = 0;
static int lineEnd = 0;
static int sorted = 0;

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
    if(lineEnd == JAGGIES_MAX_LINES) {
        return 0;
    }

    int lineID = lineEnd++;
    Line* line = lines + lineID;
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
    line->owner = owner;

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
int jaggiePoly(Point* points) {
    if(polyEnd == JAGGIES_MAX_POLYS) {
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
            if(!current) {
                return 0;
            }
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
        if(!current){
            return 0;
        }
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

int jaggieLine(int x1, int y1, int x2, int y2) {
    Line* line = addLinePrimitive(x1, y1, x2, y2, -1);
    return line != 0;
}

void jaggieClear() {
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
        // Integer line interpolation
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

    if ((x <= hitX) && (x + 1 > hitX)) {
        return 1;
    } else {
        return 0;
    }
}

int rowPixelsInLine(int x, int y, Line* l) {
    // Horizontal line special case
    if(l->y1 == l->y2){
        if(l->x1 < l->x2) {
            return (x >= l->x1) && (x <= l->x2);
        } else {
            return (x >= l->x2) && (x <= l->x1);
        }
    }

    // Check end cases
    if(l->y1 < l->y2) {
        if(y > l->y2){
            return 0;
        }
    } else {
        if(y > l->y1){
            return 0;
        }
    }

    // Integer line interpolation
    // Work up to current line
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

    // Find the number of pixels to draw in this row
    int lx = l->x;
    int err = l->err;
    int result = 0;

    while(1){
        if(lx == x) {
            return result || 1;
        }
        int e = err;
        if(e > -l->dx) {
            err -= l->dy;
            lx += l->sx;
            result++;
        }
        if(e < l->dy){
            return 0;
        }
    }

    return 0;
}

void jaggieRender(int width, int height, pixelSetter setter, void* context) {
    if(lineEnd == 0) {
        return;
    }

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

        if(lStart == lLast || ((*lStart)->y0 > y)) {
            // No lines yet, just clear scanline
            for(int x = 0; x < width; x++) {
                setter(context, x, y, 0);
            }
            continue;
        }

        while(lStart != lLast && ((*lStart)->y0 + (*lStart)->dy < y)) {
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
        int inLine = 0;
        for(int x = 0; x < width; x++) {
            for(Line** l = lStart; l < lEnd; l++) {
                int owner = (*l)->owner;
                if(owner == -1) {
                    inLine += rowPixelsInLine(x, y, *l);
                } else {
                    if(doesPixelCrossLine(x, y, *l)) {
                        Poly* poly = polys + owner;
                        poly->inside ^= 1;
                        inside += poly->inside ? 1 : -1;
                    }
                }
            }
            if(inLine > 0){
                setter(context, x, y, 1);
                inLine--;
            } else {
                setter(context, x, y, inside ? 1 : 0);
            }
        }
    }
}
