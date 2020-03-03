#include "jaggies.h"
#include <stdlib.h>

typedef jaggiePoint Point;

typedef struct Line {
    JAGGIE_INT x1, y1;
    JAGGIE_INT x2, y2;

    // Line interpolation state
    JAGGIE_INT x, y;
    JAGGIE_INT y0;
    JAGGIE_INT dx, dy;
    char sx;
    JAGGIE_INT err0, err;

    // Polygon owner, -1 for free lines.
    int owner;

#ifndef JAGGIE_SINGLE_COLOR
    JAGGIE_COLOR color;
#endif

    union {
        // This polygon line starts at a horizontal peak
        JAGGIE_INT hpeak;

        // The number of pixels to draw to complete this free line
        JAGGIE_INT lrem;
    };
} Line;

typedef struct Poly {
    JAGGIE_INT inside;
} Poly;

static Poly polys[JAGGIE_MAX_POLYS];
static Line lines[JAGGIE_MAX_LINES];
static Line* sortedLines[JAGGIE_MAX_LINES];
static JAGGIE_COLOR color;

static int polyEnd = 0;
static int lineEnd = 0;
static int sorted = 0;

static int lineCompareY0(const void* a, const void* b) {
    Line* l1 = *(Line**)a;
    Line* l2 = *(Line**)b;

    JAGGIE_INT aTop = l1->y0;
    JAGGIE_INT bTop = l2->y0;

    return aTop - bTop;
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

static Line* addLinePrimitive(JAGGIE_INT x1, JAGGIE_INT y1, JAGGIE_INT x2, JAGGIE_INT y2, int owner) {
    if(lineEnd == JAGGIE_MAX_LINES) {
        return 0;
    }

    int lineID = lineEnd++;
    Line* line = lines + lineID;
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
    line->owner = owner;
#ifndef JAGGIE_SINGLE_COLOR
    line->color = color;
#endif

    // Set up int interpolation
    if(y1 < y2) {
        line->y0 = y1;

        line->dx = x2 - x1;
        line->dy = y2 - y1;
    } else {
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

    line->err0 = (line->dx > line->dy) ? line->dx : -line->dy;
    line->err0 /= 2;

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

// First point contains vertex count and if the
// current segment is the last.
//
// To draw a 4 vertice polygon, the first point is
// {4, 1}
//
// To draw multi segment polygons, all segments
// before the last should be {x, 0}.
//
// Keep points in clock-wise order.
// Auto closes between start and end points.
JAGGIE_INT jaggiePoly(Point* points) {
    if(polyEnd == JAGGIE_MAX_POLYS) {
        return 0;
    }

    int polyID = polyEnd++;
    Poly* poly = polys + polyID;
    poly->inside = 0;

    Point* polyStart = points;

    int done = 0;
    while(!done) {
        Line* first = 0;
        Line* current = 0;
        Line* prev = 0;

        JAGGIE_INT count = polyStart->x;
        if (count < 3) {
            return 0;
        }
        JAGGIE_INT cutout = polyStart->y;
        done = !cutout;

        Point* p1 = polyStart + 1;
        Point* p2 = p1 + 1;

        for (JAGGIE_INT i = 0; i < count - 1; i++) {
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

        current = addLinePrimitive(p1->x, p1->y, first->x1, first->y1, polyID);
        if(!current){
            return 0;
        }

        setHorizontalPeak(current, prev);
        setHorizontalPeak(first, current);

        polyStart = p2;
    }

    return 1;
}

JAGGIE_INT jaggieLine(JAGGIE_INT x1, JAGGIE_INT y1, JAGGIE_INT x2, JAGGIE_INT y2) {
    Line* line = addLinePrimitive(x1, y1, x2, y2, -1);
    return line != 0;
}

void jaggieColor(JAGGIE_COLOR c) {
    color = c;
}

void jaggieClear() {
    polyEnd = 0;
    lineEnd = 0;
}

static int doesPixelCrossLine(JAGGIE_INT x, JAGGIE_INT y, Line* l) {
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

    JAGGIE_INT hitX = -1;

    // Special case for vertical line
    if(l->x1 == l->x2) {
        hitX = l->x1;
    } else {
        // Line in x - range?
        if(l->x1 < l->x2) {
            if(x + 1 < l->x1) {
                return 0;
            }
        } else {
            if(x + 1 < l->x2) {
                return 0;
            }
        }
        // Integer line interpolation
        while(l->y < y) {
            JAGGIE_INT e = l->err;
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

static JAGGIE_INT rowPixelsInLine(JAGGIE_INT x, JAGGIE_INT y, Line* l) {
    // Horizontal line special case
    if(y == l->y1 && l->y1 == l->y2){
        if(l->x1 < l->x2) {
            return (x >= l->x1) && (x <= l->x2);
        } else {
            return (x >= l->x2) && (x <= l->x1);
        }
    }

    JAGGIE_INT startX = 0;

    // Check end cases
    if(l->y1 < l->y2) {
        startX = l->x1;
        if(y > l->y2){
            return 0;
        }
    } else {
        startX = l->x2;
        if(y > l->y1){
            return 0;
        }
    }

    if(l->x1 < l->x2) {
        if(x < l->x1 || x > l->x2){
            return 0;
        }
    } else {
        if(x < l->x2 || x > l->x1){
            return 0;
        }
    }

    // Integer line interpolation
    // Work up to current line
    while(l->y < y) {
        JAGGIE_INT e = l->err;
        if(e > -l->dx) {
            l->err -= l->dy;
            l->x += l->sx;
            l->lrem++;
        }
        if(e < l->dy){
            l->err += l->dx;
            l->y++;
        }
    }

    // Make sure to draw the first pixel
    if(startX == x && l->y0 == y) {
        return 1;
    }

    // Find the number of pixels to draw in this row
    JAGGIE_INT lx = l->x;
    JAGGIE_INT err = l->err;
    JAGGIE_INT result = 0;
    
    while(1){
        if(lx == x) {
            l->x = lx;
            l->err = err;
            result += l->lrem;
            l->lrem = 0;
            return result == 0 ? 1 : result;
        }
        JAGGIE_INT e = err;
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

void jaggieRender(JAGGIE_INT width, JAGGIE_INT height, JAGGIE_COLOR bg, pixelSetter setter, void* context) {
    if(lineEnd == 0) {
        return;
    }

    // Reset line states
    for(Line* l = lines; l < lines + lineEnd; l++) {
        if(l->y1 < l->y2) {
            l->x = l->x1;
            l->y = l->y1;
        } else {
            l->x = l->x2;
            l->y = l->y2;
        }
        l->err = l->err0;
        if(l->owner == -1) {
            l->lrem = 0;
        }
    }

    // Sort lines in Y direction
    sortLines();
    Line** lStart = sortedLines;
    Line** lEnd = lStart;
    Line** lLast = lStart + lineEnd;

    // Render one scanline at a time
    for(JAGGIE_INT y = 0; y < height; y++) {

        if(lStart == lLast || ((*lStart)->y0 > y)) {
            // No lines here, just clear scanline
            for(JAGGIE_INT x = 0; x < width; x++) {
                setter(context, bg);
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

        JAGGIE_INT inside = 0;
        JAGGIE_INT inLine = 0;
        JAGGIE_COLOR lineColor;
#ifndef JAGGIE_SINGLE_COLOR
        lineColor = bg;
#else
        lineColor = color;
#endif

        for(JAGGIE_INT x = 0; x < width; x++) {

            for(Line** l = lStart; l < lEnd; l++) {
                Line* line = *l;
                int owner = line->owner;
                if(owner == -1) {
                    if(inLine == 0) {
                        inLine = rowPixelsInLine(x, y, line);
#ifndef JAGGIE_SINGLE_COLOR
                        lineColor = line->color;
#endif
                    }
                } else {
                    if(doesPixelCrossLine(x, y, line)) {
                        Poly* poly = polys + owner;
                        poly->inside ^= 1;
                        inside += poly->inside ? 1 : -1;
                    }
                }
            }
            if(inLine > 0){
                setter(context, lineColor);
                inLine--;
            } else {
                JAGGIE_COLOR polyColor = bg;
                if (inside) {
#ifndef JAGGIE_SINGLE_COLOR
                    int maxOwner = -1;
                    for (maxOwner = polyEnd - 1; maxOwner >= 0; maxOwner--)  {
                        if (polys[maxOwner].inside) {
                            break;
                        }
                    }
                    for (Line** cLine = lStart; cLine < lEnd; cLine++) {
                        int cOwner = (*cLine)->owner;
                        if (cOwner == maxOwner) {
                            polyColor = (*cLine)->color;
                            break;
                        }
                    }
#else
                    polyColor = color;
#endif
                }
                setter(context, polyColor);
            }
        }
    }
}
