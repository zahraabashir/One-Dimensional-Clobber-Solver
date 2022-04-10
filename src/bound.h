#pragma once

#include <iostream>

class Bound {
  public:
    int ups;
    bool star;

    Bound(int ups, bool star);
    Bound();

    void setMin();
    void setMax();

    static Bound min();
    static Bound max();

    Bound operator-();


};

Bound operator-(const Bound &b1, const Bound &b2);
Bound operator+(const Bound &b1, const Bound &b2);
bool operator<(const Bound &b1, const Bound &b2);
bool operator>(const Bound &b1, const Bound &b2);

bool operator==(const Bound &b1, const Bound &b2);
bool operator<=(const Bound &b1, const Bound &b2);
bool operator>=(const Bound &b1, const Bound &b2);

std::ostream &operator<<(std::ostream &os, const Bound &b);
