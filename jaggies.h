/*

Jaggies - a tiny vector graphics library

Example use:

  void setPixel(void* context, char color) {
    SomeBitmap* bmp = (SomeBitmap*) context;
    ...
  }

  SomeBitmap *bmp = createBitmapSomehow();

  // Clear the global scene..
  jaggieClear();

  jaggiePoint square[] = {
    {4, 0}, // 4 points follow
 
    {10, 10},
    {100, 10},
    {100, 100},
    {10, 100}
  };

  // ..add a polygon..
  jaggiePoly(square);

  // ..and render!
  jaggieRender(150, 150, 0, setPixel, bmp);

*/

#ifndef JAGGIE_INT
#define JAGGIE_INT short
#endif

#ifndef JAGGIE_COLOR
#define JAGGIE_COLOR unsigned char
#endif

#ifndef JAGGIE_MAX_POLYS
#define JAGGIE_MAX_POLYS 16
#endif

#ifndef JAGGIE_MAX_LINES
#define JAGGIE_MAX_LINES (JAGGIE_MAX_POLYS*3)
#endif

/*
  Point type
*/
typedef struct Point {
    JAGGIE_INT x, y;
} jaggiePoint;


/*
  Adds a polygon to the render state.
  Polygons are described by a list of points
  prefixed with a (count, last) point.

  Polygons are automatically closed, there is no need
  to repeat the first point.

  Multiple sections can optionally be added to the same
  polygon by setting last=0 in the prefix point.
  This can be used to cut a hole in a polygon.

  Returns zero on failure.
*/
JAGGIE_INT jaggiePoly(jaggiePoint* points);

/*
  Adds a line to the render state.

  The start and end pixels are included in the line drawn.

  Returns zero on failure.
*/
JAGGIE_INT jaggieLine(JAGGIE_INT x1, JAGGIE_INT y1, JAGGIE_INT x2, JAGGIE_INT y2);

/*
  Sets the render state color.
*/
void jaggieColor(JAGGIE_COLOR color);

/*
  Clears the render state
*/
void jaggieClear();

/*
  Pixel setter callback.
  
  Will always be called sequentially, row by row, from top to bottom,
  to cover the whole frame specified by the current call to `jaggieRender`.
*/
typedef void(*pixelSetter)(void* context, JAGGIE_COLOR color);

/*
  Renders the current state to a frame of size (width, height)
  using the provided pixel setter and background color.

  The generic context will be passed on to the pixel setter.
*/
void jaggieRender(JAGGIE_INT width, JAGGIE_INT height, JAGGIE_COLOR bg, pixelSetter, void* context);
