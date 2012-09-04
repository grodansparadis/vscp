/* File : jcanalci.cpp */

#include <string.h>
#include "jcanalci.h"
#define M_PI 3.14159265358979323846

/* Move the shape to a new location */
void Shape::move(double dx, double dy) {
  x += dx;
  y += dy;
}

int Shape::nshapes = 0;

double Circle::area(void) {
  return M_PI*radius*radius;
}

double Circle::perimeter(void) {
  return 2*M_PI*radius;
}

double Square::area(void) {
  return width*width;
}

double Square::perimeter(void) {
  return 4*width;
}


void Circle::setSuper( tttest *p, int val ) 
{ 
	p->bSuper = val; 
	strcpy( p->strtest, "this is a wundefull test....\0");
}

tttest * Circle::new_tttest( void )
{
	tttest *p = new tttest;
	return p;
}

int Circle::test( tttest *p )
{
	p->bSuper = 88;
	return p->bSuper;
}


int Circle::test2( Shape *p )
{
	return 99;	
}
