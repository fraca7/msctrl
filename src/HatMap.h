
#ifndef _MSCTRL_HATMAP_H
#define _MSCTRL_HATMAP_H

#include <src/MasterSystem.h>
#include <src/Controller.h>

namespace MSCtrl
{
  class HatMap : public Controller::Listener
  {
  public:
    HatMap(MasterSystem&);

    void add_to(Controller&) override;
    void on_button_state(Controller&, Controller::Button, bool) override;

  private:
    MasterSystem& m_ms;
    uint8_t m_state;
  };
}

#endif /* _MSCTRL_HATMAP_H */
