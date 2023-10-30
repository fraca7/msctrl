
#ifndef _MSCTRL_MASTERSYSTEM_H
#define _MSCTRL_MASTERSYSTEM_H

#include <map>
#include <string>

namespace MSCtrl
{
  class MasterSystem
  {
  public:
    enum class Button {
      B1,
      B2,
      Up,
      Down,
      Left,
      Right
    };

    MasterSystem();
    ~MasterSystem();

    void set_button_state(Button, bool);

    static std::string button_name(Button);
    static Button button_from_name(const std::string&);

#ifdef ENABLE_GPIO
    void set_gpio_map(Button, unsigned);

  private:
    std::map<Button, unsigned> m_map;
#endif
  };
}

#endif /* _MSCTRL_MASTERSYSTEM_H */
