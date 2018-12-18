```
    |               o
    |,---.,---.,---..,---.,---.
    |,---||   ||   |||---'`---.
`---'`---^`---|`---|``---'`---'
          `---'`---'
```

## Jaggies - a tiny vector graphics library

Jaggies is a tiny C library for drawing filled polygons
and lines to a monochrome destination.

Jaggies uses a global scene, which is built up by 
adding polygons and lines, and is then rendered
in one continuous flow, one pixel at a time, row by row.

This makes it possible to draw basic vector graphics 
to destinations without random access frame buffer 
access. No frame buffer is used internally either,
keeping memory requirements down.

Jaggies has no external dependencies except for the
standard C library, uses only integer math and makes
no dynamic memory allocations.
This makes Jaggies well suited for running on micro
controllers with very little memory.

