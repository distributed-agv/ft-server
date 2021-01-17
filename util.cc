#include "util.h"

IntPair::IntPair() : x(0), y(0) {}

IntPair::IntPair(int x, int y) : x(x), y(y) {}

bool IntPair::operator<(const IntPair &rhs) const {
  return x < rhs.x || (x == rhs.x && y < rhs.y);
}

bool IntPair::operator>(const IntPair &rhs) const {
  return x > rhs.x || (x == rhs.x && y > rhs.y);
}

bool IntPair::operator==(const IntPair &rhs) const {
  return x == rhs.x && y == rhs.y;
}

bool IntPair::operator!=(const IntPair &rhs) const {
  return x != rhs.x || y != rhs.y;
}

IntPair IntPair::operator+(const IntPair &rhs) const {
  return IntPair(x + rhs.x, y + rhs.y);
}

IntPair IntPair::operator-(const IntPair &rhs) const {
  return IntPair(x - rhs.x, y - rhs.y);
}

int IntPair::DotProduct(const IntPair &rhs) const {
  return x * rhs.x + y * rhs.y;
}

bool IntPair::Validate(const IntPair &boundary) const {
  return x >= 0 && x < boundary.x && y >= 0 && y < boundary.y;
}

const std::vector<IntPair> kOffsets = {
  IntPair(0, 0),
  IntPair(0, -1),
  IntPair(0, 1),
  IntPair(1, 0),
  IntPair(-1, 0),
};