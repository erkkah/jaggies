/*

Jaggies - a tiny vector graphics library

Example use:

  void setPixel(void* context, int x, int y, int color) {
    SomeBitmap* bmp = (SomeBitmap*) context;
    ...
  }

  SomeBitmap *bmp = createBitmapSomehow();

  // Clear the global scene..
  jaggieClear();

  jaggiePoint square[] = {
    {10, 10},
    {100, 10},
    {100, 100},
    {10, 100},
    {-1, -1}
  };

  // ..add a polygon..
  jaggiePoly(square);

  // ..and render!
  jaggieRender(150, 150, setPixel, bmp);

*/

#ifndef JAGGIE_INT
#define JAGGIE_INT short
#endif

#ifndef JAGGIES_MAX_POLYS
#define JAGGIES_MAX_POLYS 16
#endif

#ifndef JAGGIES_MAX_LINES
#define JAGGIES_MAX_LINES (JAGGIES_MAX_POLYS*3)
#endif

// Point type
typedef struct Point {
    JAGGIE_INT x, y;
} jaggiePoint;

// Add a polygon to the render state.
// End the list with a {-1, -1} point.
// Several sections can be added to the same
// polygon by separating the sections with {-2, -2}.
// This can be used to cut a hole in a polygon.
// The last coordinate in each segment connects
// back to the first coordinate.
//
// Returns zero on failure.
JAGGIE_INT jaggiePoly(jaggiePoint* points);

// Add a line to the render state.
// The start and end coordinates are included in the line drawn.
//
// Returns zero on failure.
JAGGIE_INT jaggieLine(JAGGIE_INT x1, JAGGIE_INT y1, JAGGIE_INT x2, JAGGIE_INT y2);

// Clear the render state
void jaggieClear();

// Pixel setter callback. Color is 0 or 1.
typedef void(*pixelSetter)(void* context, JAGGIE_INT x, JAGGIE_INT y, JAGGIE_INT color);

// Render the state using the provided pixel setter
void jaggieRender(JAGGIE_INT width, JAGGIE_INT height, pixelSetter, void* context);
