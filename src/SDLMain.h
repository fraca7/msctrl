
#ifndef _MSCTRL_SDLMAIN_H
#define _MSCTRL_SDLMAIN_H

#include <list>
#include <memory>

#include <src/Controller.h>

namespace MSCtrl
{
  class SDLMain
  {
  public:
    SDLMain();
    virtual ~SDLMain();

    void loop();

    virtual bool on_controller_added(const std::string& name) = 0;
    virtual void on_controller_open(Controller&) = 0;

  private:
    std::list<std::unique_ptr<Controller>> m_controllers;
  };
}

#endif /* _MSCTRL_SDLMAIN_H */
