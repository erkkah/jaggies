typedef struct Point {
    int x, y;
} tinyPoint;

void tinyPoly(tinyPoint* points);
void tinyLine(int x1, int y1, int x2, int y2);
void tinyClear();

typedef void(*pixelSetter)(void* context, int x, int y, int color);

void tinyRender(int width, int height, pixelSetter, void* context);
