/* File : jcanalci.h */

typedef struct {
	unsigned int bSuper;
	unsigned int bMini;
	char strtest[128];
} tttest;

class Shape {
public:
  Shape() {
    nshapes++;
  }
  virtual ~Shape() {
    nshapes--;
  };
  double  x, y;   
  void    move(double dx, double dy);
  virtual double area(void) = 0;
  virtual double perimeter(void) = 0;
  static  int nshapes;
};

class Circle : public Shape {
private:
  double radius;
public:
  Circle(double r) : radius(r) { };
  virtual double area(void);
  virtual double perimeter(void);

  tttest *new_tttest( void );
//devItem *new_devItem( void );

  int test( tttest *p );
  int test2( Shape * p );
  void setSuper( tttest *p, int val );
};

class Square : public Shape {
private:
  double width;
public:
  Square(double w) : width(w) { };
  virtual double area(void);
  virtual double perimeter(void);
};




  
