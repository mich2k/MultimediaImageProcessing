#include "canvas.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

canvas *canvas_create(canvas *this, int width, int height) 
{
    assert(width > 0 && height > 0);
    this->width_ = width;
    this->height_ = height;
    int size = this->width_ * this->height_;
    this->ptr_ = memset(malloc(size), ' ', size);
    return this;
}

canvas *canvas_destroy(canvas *this)
{
    free(this->ptr_);
    return this;
}

canvas *new_canvas(int width, int height) 
{
    return canvas_create(malloc(sizeof(canvas)), width, height);
}

void delete_canvas(canvas *this) 
{
    free(canvas_destroy(this));
}

void canvas_set(canvas *this, int x, int y, char c) 
{
    if (x >= 0 && x < this->width_ && y >= 0 && y < this->height_)
        this->ptr_[y*this->width_ + x] = c;
}

void canvas_line(canvas *this, int x0, int y0, int x1, int y1, char c)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2; /* error value e_xy */

    for (;;) {  /* loop */
        canvas_set(this, x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
        if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
    }
}

void canvas_rectangle(canvas *this, int x0, int y0, int x1, int y1, char c)
{
    canvas_line(this, x0, y0, x1, y0, c);
    canvas_line(this, x0, y1, x1, y1, c);
    canvas_line(this, x0, y0, x0, y1, c);
    canvas_line(this, x1, y0, x1, y1, c);
}

void canvas_circle(canvas *this, int xm, int ym, int r, char c)
{
    int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
    do {
        canvas_set(this, xm - x, ym + y, c); /*   I. Quadrant */
        canvas_set(this, xm - y, ym - x, c); /*  II. Quadrant */
        canvas_set(this, xm + x, ym - y, c); /* III. Quadrant */
        canvas_set(this, xm + y, ym + x, c); /*  IV. Quadrant */
        r = err;
        if (r <= y) err += ++y * 2 + 1;           /* e_xy+e_y < 0 */
        if (r > x || err > y) err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
    } while (x < 0);
}

void canvas_out(canvas *this, FILE *f) 
{
    for (int y = 0; y < this->height_; ++y) {
        for (int x = 0; x < this->width_; ++x) {
            fputc(this->ptr_[y*this->width_ + x], f);
        }
        fputc('\n', f);
    }
}
