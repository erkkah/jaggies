# Jaggies - a tiny vector graphics library

```asciiart
    |               o
    |,---.,---.,---..,---.,---.
    |,---||   ||   |||---'`---.
`---'`---^`---|`---|``---'`---'
          `---'`---'
```

Jaggies is a tiny C library for drawing filled polygons
and lines to a monochrome destination.

Jaggies uses a global state, which is built up by
adding polygons and lines, and is then rendered
in one continuous flow, one pixel at a time, row by row.

This makes it possible to draw basic vector graphics
to destinations without random access frame buffer
access. No frame buffer is used internally either,
keeping memory requirements low.

Jaggies has no external dependencies except for the
standard C library, uses only integer math and makes
no dynamic memory allocations.
This makes Jaggies well suited for running on micro
controllers with limited memory.

Jaggies is distributed under the MIT license.

## Getting started

Basic example:

```C
void setPixel(void* context, JAGGIE_INT x, JAGGIE_INT y, char color) {
    SomeBitmap* bmp = (SomeBitmap*) context;
    // implementation here!
}

SomeBitmap *bmp = createBitmapSomehow();

// Clear the global state
jaggieClear();

// A square
jaggiePoint square[] = {
    {10, 10},
    {100, 10},
    {100, 100},
    {10, 100},
    {-1, -1}
};

// Add a polygon
jaggiePoly(square);

// Render using `setPixel` above
jaggieRender(150, 150, setPixel, bmp);
```

The full four function API is documented in [jaggies.h](jaggies.h).

Check out the example in the [test](test) folder.

> NOTE: The example uses the awesome [tigr](https://github.com/erkkah/tigr) library, which is included as a submodule, but Jaggies itself has no external dependencies.

## Memory requirement tweaks

Since required structures are statically allocated, the overall memory requirement is defined by the coordinate type and the maximum number of lines and polygons in the global render state.

These are set to relatively low levels (`short int` coordinates, 16 polygons and 48 lines) by default, and can be overridden by setting the `JAGGIE_INT`, `JAGGIE_MAX_POLYS` and `JAGGIE_MAX_LINES` preprocessor defines.

## Known issues

* Jaggies is designed for limited memory situations, it is not your fastest pixel pushing friend. But that's OK.

* Jaggies uses a global state which makes drawing several scenes at the same time impossible. Also fine.
