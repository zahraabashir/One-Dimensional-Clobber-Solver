#pragma once

struct Bound {
    int ups;
    bool star;

    Bound(int ups, bool star);
    Bound();

    void setMin();
    void setMax();

    Bound operator-(const Bound &b);


};

Bound operator-(const Bound &b1, const Bound &b2);
Bound operator+(const Bound &b1, const Bound &b2);
bool operator<(const Bound &b1, const Bound &b2);
bool operator>(const Bound &b1, const Bound &b2);
