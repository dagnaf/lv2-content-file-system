#include <cstring>
#include "file.h"

void File::init(const char *s, int d) {
  strcpy(name, s);
  pt = d;
}
