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

// Point type
typedef struct Point {
    int x, y;
} jaggiePoint;

// Add a polygon to the render state.
// End the list with a {-1, -1} point.
// Several sections can be added to the same
// polygon by separating the sections with {-2, -2}.
// This can be used to cut a hole in a polygon.
// The last coordinate in each segment connects
// back to the first coordinate.
int jaggiePoly(jaggiePoint* points);

// Add a line to the render state.
// The start and end coordinates are included in the line drawn.
int jaggieLine(int x1, int y1, int x2, int y2);

// Clear the render state
void jaggieClear();

// Pixel setter callback. Color is 0 or 1.
typedef void(*pixelSetter)(void* context, int x, int y, int color);

// Render the state using the provided pixel setter
void jaggieRender(int width, int height, pixelSetter, void* context);
