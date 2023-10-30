
#ifndef _MSCTRL_CLPARSER_H
#define _MSCTRL_CLPARSER_H

#include <stdexcept>

#include <src/Controller.h>
#include <src/MasterSystem.h>

namespace MSCtrl
{
  class CLParser
  {
  public:
    class usage_exception : public std::exception {
    public:
      using std::exception::exception;
    };

    class ConfigurationTarget
    {
    public:
      virtual void add_map(Controller::Listener*) = 0;
      virtual void set_trigger_threshold(float) = 0;
    };

    CLParser();

    void parse(MasterSystem&, ConfigurationTarget&, int argc, char* argv[]);
    void usage();
  };
}

#endif /* _MSCTRL_CLPARSER_H */
