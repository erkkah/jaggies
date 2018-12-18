/*

tinyvec - a tiny lib for drawing vector graphics to a monochrome destination.
Supports filled polygons and lines.

The scene is prepared by adding polygons and lines, and is then rendered
in one continuous flow, one pixel at a time.

Each call to `tinyRender` draws all pixels in the destination.
The pixels are guaranteed to be drawn in sequence, row by row,
by calling the provided pixel setter.

Example:

  void setPixel(void* context, int x, int y, int color) {
    SomeBitmap* bmp = (SomeBitmap*) context;
    ...
  }

  SomeBitmap *bmp = createBitmapSomehow();

  tinyPoint square[] = {
    {10, 10},
    {100, 10},
    {100, 100},
    {10, 100},
    {-1, -1}
  };

  tinyPoly(square);
  tinyRender(150, 150, setPixel, bmp);

*/

// Point type
typedef struct Point {
    int x, y;
} tinyPoint;

// Add a polygon to the render state.
int tinyPoly(tinyPoint* points);

// Add a line to the render state
int tinyLine(int x1, int y1, int x2, int y2);

// Clear the render state
void tinyClear();

// Pixel setter callback. Color is 0 or 1.
typedef void(*pixelSetter)(void* context, int x, int y, int color);

// Render the state using the provided pixel setter
void tinyRender(int width, int height, pixelSetter, void* context);
