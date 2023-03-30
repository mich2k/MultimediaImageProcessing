#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>

#include "canvas.h"
#include "string.h"

/*
type_of_object parameter_1 parameter_2 ... parameter_n

rectangle 0 0 79 24 *
point 5 15 ?
circle 10 10 4 @
circle 70 10 4 @
line 40 15 40 20 |
*/

void draw_file(FILE *f, canvas *c)
{
	char type[30];
	while (fscanf(f, "%29s", type) == 1) {
		if (strcmp(type, "point") == 0) {
			int x, y;
			char ch;
			fscanf(f, "%d%d %c", &x, &y, &ch);
			canvas_set(c, x, y, ch);
		}
		else if (strcmp(type, "line") == 0) {
			int x0, y0, x1, y1;
			char ch;
			fscanf(f, "%d%d%d%d %c", &x0, &y0, &x1, &y1, &ch);
			canvas_line(c, x0, y0, x1, y1, ch);
		}
		else if (strcmp(type, "rectangle") == 0) {
			int x0, y0, x1, y1;
			char ch;
			fscanf(f, "%d%d%d%d %c", &x0, &y0, &x1, &y1, &ch);
			canvas_rectangle(c, x0, y0, x1, y1, ch);
		}
		else if (strcmp(type, "circle") == 0) {
			int xm, ym, r;
			char ch;
			fscanf(f, "%d%d%d %c", &xm, &ym, &r, &ch);
			canvas_circle(c, xm, ym, r, ch);
		}
	}
}

typedef struct shape {
	int x_, y_;
	char ch_;
	char* name_;


} shape;

shape* shape_create(shape* this, int x, int y, char ch, const char* name) {
	this->x_ = x;
	this->y_ = y;
	this->ch_ = ch;
	this->name_ = srtcpy(malloc(strlen(name) + 1), name); // +1 per il terminatore
	return this;
}

shape* shape_destroy(shape* this) {
	free(this->name_);
	return this;
}

shape* new_shape(int x, int y, char ch, const char* name) {
	return shape_create(malloc(sizeof(shape)), x, y, ch, name);
}
void delete_shape(shape* this) {
	free(shape_destroy(this));
}

void shape_draw(const shape* this, canvas* c) {
	canvas_set(c, this->x_, this->y_, this->ch_);
}



int main(void)
{
	canvas* c = new_canvas(80, 25);

	draw_file(stdin, c);

	canvas_out(c, stdout);
	delete_canvas(c);
}