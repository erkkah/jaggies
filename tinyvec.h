/*

tinyvec - a tiny lib for writing vector graphics to a monochrome destination.
Supports filled polygons and lines.

The scene is prepared by adding polygons and lines, and is then rendered
in one continuous flow, one pixel at a time.

*/

typedef struct Point {
    int x, y;
} tinyPoint;

int tinyPoly(tinyPoint* points);
int tinyLine(int x1, int y1, int x2, int y2);
void tinyClear();

typedef void(*pixelSetter)(void* context, int x, int y, int color);

void tinyRender(int width, int height, pixelSetter, void* context);
