
#ifndef _MSCTRL_BUTTONMAP_H
#define _MSCTRL_BUTTONMAP_H

#include <set>

#include <src/Controller.h>
#include <src/MasterSystem.h>

namespace MSCtrl
{
  class ButtonMap : public Controller::Listener
  {
  public:
    enum class Button {
      B1,
      B2
    };

    ButtonMap(MasterSystem& ms, Button dst);

    void add_source_button(Controller::Button);

    void add_to(Controller&) override;
    void on_button_state(Controller&, Controller::Button, bool) override;

  private:
    MasterSystem& m_ms;
    Button m_dst;
    std::set<Controller::Button> m_src;
  };
}

#endif /* _MSCTRL_BUTTONMAP_H */
