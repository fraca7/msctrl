
#include "utils.h"

using namespace std;

namespace MSCtrl
{
  void split_string(const string& s, char sep, const function<void (const string&)>& cb)
  {
    int state = 0;
    ostringstream current;
    for (auto c : s) {
      switch (state) {
        case 0: // First character
          if (c == sep) {
            cb("");
            state = 1;
          } else {
            current.write(&c, 1);
            state = 2;
          }
          break;
        case 1: // After separator
          if (c == sep) {
            cb("");
          } else {
            current.write(&c, 1);
            state = 2;
          }
          break;
        case 2: // After other character
          if (c == sep) {
            cb(current.str());
            current.str("");
            state = 1;
          } else {
            current.write(&c, 1);
          }
          break;
      }
    }

    switch (state) {
      case 0:
        break;
      case 1:
        cb("");
        break;
      case 2:
        cb(current.str());
        break;
    }
  }

  void split_string(const string& s, char sep, const function<void (unsigned, const string&)>& cb)
  {
    unsigned int index = 0;
    split_string(s, sep, [&](const string& part) { cb(index++, part); });
  }
}
