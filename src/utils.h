
#ifndef _MSCTRL_UTILS_H
#define _MSCTRL_UTILS_H

#include <string>
#include <functional>
#include <sstream>

namespace MSCtrl
{
  void split_string(const std::string& s, char sep, const std::function<void (const std::string&)>& cb);
  void split_string(const std::string& s, char sep, const std::function<void (unsigned, const std::string&)>& cb);

  template <typename InputIterator> std::string join_strings(const std::string& sep, InputIterator begin, InputIterator end) {
    bool first = true;
    std::ostringstream oss;
    while (begin != end) {
      if (!first)
        oss << sep;
      first = false;
      oss << *begin++;
    }
    return oss.str();
  }
}

#endif /* _MSCTRL_UTILS_H */
