#include "bound.h"
#include <limits>
//#include <iostream>

using namespace std;

Bound::Bound(int ups, bool star) {
    this->ups = ups;
    this->star = star;
}

Bound::Bound() {
    this->ups = 0;
    this->star = false;
}

void Bound::setMin() {
    //ups = numeric_limits<int>::min() + 1;
    ups = -10000;
    star = false;
}

void Bound::setMax() {
    //ups = numeric_limits<int>::max() - 1;
    ups = 10000;
    star = false;
}

Bound Bound::min() {
    Bound b;
    b.setMin();
    return b;
}

Bound Bound::max() {
    Bound b;
    b.setMax();
    return b;
}

Bound Bound::operator-() {
    return Bound(-ups, star);
}

Bound operator-(const Bound &b1, const Bound &b2) {
    return Bound(b1.ups - b2.ups, b1.star ^ b2.star);
}

Bound operator+(const Bound &b1, const Bound &b2) {
    return Bound(b1.ups + b2.ups, b1.star ^ b2.star);
}

bool operator<(const Bound &b1, const Bound &b2) {
    Bound b3 = b1 - b2;
    int val = b3.ups + (b3.star ? 1 : 0);
    return val < 0;
}

bool operator>(const Bound &b1, const Bound &b2) {
    Bound b3 = b1 - b2;
    int val = b3.ups - (b3.star ? 1 : 0);
    return val > 0;
}

bool operator==(const Bound &b1, const Bound &b2) {
    return (b1.ups == b2.ups) && (b1.star == b2.star);
}

bool operator<=(const Bound &b1, const Bound &b2) {
    return (b1 == b2) || (b1 < b2);
}

bool operator>=(const Bound &b1, const Bound &b2) {
    return (b1 == b2) || (b1 > b2);
}

std::ostream &operator<<(std::ostream &os, const Bound &b) {
    os << "|" << b.ups << " " << b.star << "|";
    return os;
}
