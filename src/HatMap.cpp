
#include <spdlog/spdlog.h>

#include "HatMap.h"

namespace MSCtrl
{
  HatMap::HatMap(MasterSystem& ms)
    : m_ms(ms),
      m_state(0)
  {
  }

  void HatMap::add_to(Controller& ctrl)
  {
    spdlog::info("Add DPad map to {}", ctrl.name());

    Controller::Listener::add_to(ctrl);
  }

  void HatMap::on_button_state(Controller& ctrl, Controller::Button btn, bool state)
  {
    // Toggle only state changes, in case there's something else (axis) mapping to the MS dpad
    switch (btn) {
      case Controller::Button::DPadLeft:
        spdlog::debug("DPad left change for {}: {}", ctrl.name(), state);
        if ((((m_state & 0x01) == 0) && state) || (((m_state & 0x01) != 0) && !state))
          m_ms.set_button_state(MasterSystem::Button::Left, state);
        m_state = (m_state & ~0x01) | (state ? 0x01 : 0x00);
        break;
      case Controller::Button::DPadRight:
        spdlog::debug("DPad right change for {}: {}", ctrl.name(), state);
        if ((((m_state & 0x02) == 0) && state) || (((m_state & 0x02) != 0) && !state))
          m_ms.set_button_state(MasterSystem::Button::Right, state);
        m_state = (m_state & ~0x02) | (state ? 0x02 : 0x00);
        break;
      case Controller::Button::DPadUp:
        spdlog::debug("DPad up change for {}: {}", ctrl.name(), state);
        if ((((m_state & 0x04) == 0) && state) || (((m_state & 0x04) != 0) && !state))
          m_ms.set_button_state(MasterSystem::Button::Up, state);
        m_state = (m_state & ~0x04) | (state ? 0x04 : 0x00);
        break;
      case Controller::Button::DPadDown:
        spdlog::debug("DPad down change for {}: {}", ctrl.name(), state);
        if ((((m_state & 0x08) == 0) && state) || (((m_state & 0x08) != 0) && !state))
          m_ms.set_button_state(MasterSystem::Button::Down, state);
        m_state = (m_state & ~0x08) | (state ? 0x08 : 0x00);
        break;
      default:
        break;
    }
  }
}
