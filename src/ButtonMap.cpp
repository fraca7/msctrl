
#include <spdlog/spdlog.h>

#include "ButtonMap.h"
#include "utils.h"

namespace MSCtrl
{
  ButtonMap::ButtonMap(MasterSystem& ms, ButtonMap::Button dst)
    : m_ms(ms),
      m_dst(dst),
      m_src()
  {
  }

  void ButtonMap::add_source_button(Controller::Button btn)
  {
    m_src.insert(btn);
  }

  void ButtonMap::add_to(Controller& ctrl)
  {
    spdlog::info("Add button mapping {} -> {} to {}", join_strings(",", m_src.begin(), m_src.end()), (m_dst == Button::B1) ? "B1" : "B2", ctrl.name());

    Controller::Listener::add_to(ctrl);
  }

  void ButtonMap::on_button_state(Controller& ctrl, Controller::Button btn, bool state)
  {
    if (m_src.find(btn) != m_src.end()) {
      switch (m_dst) {
        case Button::B1:
          spdlog::debug("{} B1 (from {} {})", (state ? "Press" : "Release"), ctrl.name(), Controller::button_name(btn));
          m_ms.set_button_state(MasterSystem::Button::B1, state);
          break;
        case Button::B2:
          spdlog::debug("{} B2 (from {} {})", (state ? "Press" : "Release"), ctrl.name(), Controller::button_name(btn));
          m_ms.set_button_state(MasterSystem::Button::B2, state);
          break;
      }
    }
  }
}
