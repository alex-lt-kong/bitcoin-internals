#include <math.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <float.h>
#include <limits.h>
#include "../01_finite-field/field-element.h"

#ifndef FieldElementPoint_H
#define FieldElementPoint_H

using namespace std;

class FieldElementPoint {

  private:
    FieldElement* a = nullptr;
    FieldElement* b = nullptr;
    FieldElement* x = nullptr;
    FieldElement* y = nullptr;
  public:
    FieldElementPoint(FieldElement x, FieldElement y, FieldElement a, FieldElement b);
    bool operator==(const FieldElementPoint& other) const;
    FieldElementPoint operator+(const FieldElementPoint& other);
    string toString();
};

FieldElementPoint::FieldElementPoint(FieldElement x, FieldElement y, FieldElement a, FieldElement b) {  
  this->x = &x;
  this->y = &y;
  this->a = &a;
  this->b = &b;
  if (this->x == nullptr && this->y == nullptr) { return; }
  if (this->y->power(2) != this->x->power(3) + (*(this->a) * *(this->x)) + *(this->b)) {
    throw std::invalid_argument(
      "Point (" + this->x->toString() + ", " + this->y->toString() +") not on the curve"
    );
  }
}


bool FieldElementPoint::operator==(const FieldElementPoint& other) const
{
  // ICYW: This is overloading, not overriding
  return (
    this->a == other.a && this->b == other.b && 
    this->x == other.x && this->y == other.y
  );
}
/*
FieldElementPoint FieldElementPoint::operator+(const FieldElementPoint& other)
{
  // ICYW: This is overloading, not overriding lol
  if (*this == other || this->y == 0) {
    return FieldElementPoint(FLT_MAX, FLT_MAX, this->a, this->b);
  }
  if (this->a != other.a || this->b != other.b) {
    throw std::invalid_argument("Two FieldElementPoints are not on the same curve");
  }
  // FLT_MAX represents infinity
  if (this->x == FLT_MAX) { return other; }
  if (other.x == FLT_MAX) { return *this; }
  if (this->x == other.x && this->y != other.y) {
    return FieldElementPoint(FLT_MAX, FLT_MAX, this->a, this->b);//FieldElementPoint at infinity
  }

  float slope = 0;
  if (this->x == other.x && this->y == other.y) {
    // P1 == P2, need some calculus to derive this formula
    slope = (3 * this->x * this->x + this->a) / (this->y * 2);
  } else {
    // general case
    slope = (other.y - this->y) / (other.x - this->x);
  }
  float x3 = slope * slope - this->x - other.x;
  float y3 = slope * (this->x - x3) - this->y;
  return FieldElementPoint(x3, y3, this->a, this->b);
}
*/
string FieldElementPoint::toString() {
  return "FieldElementPoint(" + to_string(this->x->num) + ", " + to_string(this->y->num) + ")_"
                              + to_string(this->a->num) +  "_" + to_string(this->b->num)
                              + " FieldElement(" + to_string(this->x->prime) + ")";

}

#endif