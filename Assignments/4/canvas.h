#if !defined CANVAS_H
#define CANVAS_H

#include <stdio.h>

typedef struct canvas {
    int width_, height_;
    char *ptr_;
} canvas;

canvas *canvas_create(canvas *this, int width, int height);
canvas *canvas_destroy(canvas *this);
canvas *new_canvas(int width, int height);
void delete_canvas(canvas *this);
void canvas_set(canvas *this, int x, int y, char c);
void canvas_line(canvas *this, int x0, int y0, int x1, int y1, char c);
void canvas_rectangle(canvas *this, int x0, int y0, int x1, int y1, char c);
void canvas_circle(canvas *this, int xm, int ym, int r, char c);
void canvas_out(canvas *this, FILE *f);

#endif // CANVAS_H