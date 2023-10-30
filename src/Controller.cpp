
#include <stdexcept>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "Controller.h"

using namespace std;

namespace MSCtrl
{
  void Controller::Listener::add_to(Controller& ctrl)
  {
    ctrl.add_listener(this);
  }

  Controller::Controller(int index)
    : m_handle(SDL_GameControllerOpen(index)),
      m_trigger_threshold(0.5f),
      m_listeners(),
      m_last_left(0.0f),
      m_last_right(0.0f)
  {
    if (!m_handle)
      throw runtime_error(fmt::format("Error opening controller #{}: {}", index, SDL_GetError()));

    if (SDL_GameControllerHasSensor(m_handle, SDL_SENSOR_GYRO)) {
      if (SDL_GameControllerSetSensorEnabled(m_handle, SDL_SENSOR_GYRO, SDL_TRUE) < 0)
        throw runtime_error(fmt::format("Cannot enable gyro: {}", SDL_GetError()));
      spdlog::info("Gyro enabled on {}", name());
    } else {
      spdlog::warn("Controller {} has no gyro", name());
    }
  }

  Controller::~Controller()
  {
    SDL_GameControllerClose(m_handle);
  }

  void Controller::set_trigger_threshold(float value)
  {
    if (value <= 0.0f)
      throw runtime_error(fmt::format("Value {} for trigger threshold is <= 0", value));
    if (value >= 1.0f)
      throw runtime_error(fmt::format("Value {} for trigger threshold is >= 1", value));

    m_trigger_threshold = value;
    spdlog::debug("Trigger threshold for {}: {:.2f}", name(), value);
  }

  void Controller::add_listener(Controller::Listener* listener)
  {
    m_listeners.push_back(listener);
  }

  void Controller::remove_listener(Controller::Listener* listener)
  {
    m_listeners.remove(listener);
  }

  string Controller::name() const
  {
    return SDL_GameControllerName(m_handle);
  }

  bool Controller::matches(SDL_JoystickID id) const
  {
    return (id == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(m_handle)));
  }

  void Controller::on_button_press(uint8_t btn)
  {
    Button b;
    if (map_button(btn, b)) {
      for (auto listener : m_listeners)
        listener->on_button_state(*this, b, true);
    }
  }

  void Controller::on_button_release(uint8_t btn)
  {
    Button b;
    if (map_button(btn, b)) {
      for (auto listener : m_listeners)
        listener->on_button_state(*this, b, false);
    }
  }

  void Controller::on_axis_motion(uint8_t axis, int16_t value)
  {
    float fvalue = 1.0f * value / SDL_JOYSTICK_AXIS_MAX;
    switch (axis) {
      case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        if ((m_last_left < m_trigger_threshold) && (fvalue >= m_trigger_threshold)) {
          spdlog::debug("Left trigger for {} above threshold", name());
          for (auto listener : m_listeners)
            listener->on_button_state(*this, Controller::Button::LeftTrigger, true);
        } else if ((m_last_left >= m_trigger_threshold) && (fvalue < m_trigger_threshold)) {
          spdlog::debug("Left trigger for {} below threshold", name());
          for (auto listener : m_listeners)
            listener->on_button_state(*this, Controller::Button::LeftTrigger, false);
        }
        m_last_left = fvalue;
        break;
      case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        if ((m_last_right < m_trigger_threshold) && (fvalue >= m_trigger_threshold)) {
          spdlog::debug("Right trigger for {} above threshold", name());
          for (auto listener : m_listeners)
            listener->on_button_state(*this, Controller::Button::RightTrigger, true);
        } else if ((m_last_right >= m_trigger_threshold) && (fvalue < m_trigger_threshold)) {
          spdlog::debug("Right trigger for {} below threshold", name());
          for (auto listener : m_listeners)
            listener->on_button_state(*this, Controller::Button::RightTrigger, false);
        }
        m_last_right = fvalue;
        break;
      default:
        break;
    }

    Axis a;
    if (map_axis(axis, a)) {
      for (auto listener : m_listeners)
        listener->on_axis_motion(*this, a, fvalue);
    }
  }

  void Controller::on_gyro_update(uint32_t timestamp, float dx, float dy, float dz)
  {
    for (auto& listener : m_listeners)
      listener->on_gyro_update(*this, timestamp, dx, dy, dz);
  }

  bool Controller::map_button(uint8_t src, Controller::Button& dst)
  {
    switch (src) {
      case SDL_CONTROLLER_BUTTON_A:
        dst = Button::A;
        break;
      case SDL_CONTROLLER_BUTTON_B:
        dst = Button::B;
        break;
      case SDL_CONTROLLER_BUTTON_X:
        dst = Button::X;
        break;
      case SDL_CONTROLLER_BUTTON_Y:
        dst = Button::Y;
        break;
      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        dst = Button::LeftShoulder;
        break;
      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        dst = Button::RightShoulder;
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        dst = Button::DPadLeft;
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        dst = Button::DPadRight;
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_UP:
        dst = Button::DPadUp;
        break;
      case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        dst = Button::DPadDown;
        break;
      default:
        return false;
    }

    return true;
  }

  bool Controller::map_axis(uint8_t src, Controller::Axis& dst)
  {
    switch (src) {
      case SDL_CONTROLLER_AXIS_LEFTX:
        dst = Axis::LeftX;
        break;
      case SDL_CONTROLLER_AXIS_RIGHTX:
        dst = Axis::RightX;
        break;
      case SDL_CONTROLLER_AXIS_LEFTY:
        dst = Axis::LeftY;
        break;
      case SDL_CONTROLLER_AXIS_RIGHTY:
        dst = Axis::RightY;
        break;
      case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        dst = Axis::LeftTrigger;
        break;
      case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        dst = Axis::RightTrigger;
        break;
      default:
        return false;
    }

    return true;
  }

  string Controller::button_name(Controller::Button btn)
  {
    switch (btn) {
      case Controller::Button::A:
        return "A";
      case Controller::Button::B:
        return "B";
      case Controller::Button::X:
        return "X";
      case Controller::Button::Y:
        return "Y";
      case Controller::Button::LeftShoulder:
        return "LS";
      case Controller::Button::RightShoulder:
        return "RS";
      case Controller::Button::LeftTrigger:
        return "LT";
      case Controller::Button::RightTrigger:
        return "RT";
      case Controller::Button::DPadLeft:
        return "L";
      case Controller::Button::DPadRight:
        return "R";
      case Controller::Button::DPadUp:
        return "U";
      case Controller::Button::DPadDown:
        return "D";
    }

    return "UNK";
  }

  Controller::Button Controller::button_from_name(const string& name)
  {
    if (name == "A")
      return Controller::Button::A;
    if (name == "B")
      return Controller::Button::B;
    if (name == "X")
      return Controller::Button::X;
    if (name == "Y")
      return Controller::Button::Y;
    if (name == "LS")
      return Controller::Button::LeftShoulder;
    if (name == "RS")
      return Controller::Button::RightShoulder;
    if (name == "LT")
      return Controller::Button::LeftTrigger;
    if (name == "RT")
      return Controller::Button::RightTrigger;
    if (name == "L")
      return Controller::Button::DPadLeft;
    if (name == "R")
      return Controller::Button::DPadRight;
    if (name == "U")
      return Controller::Button::DPadUp;
    if (name == "D")
      return Controller::Button::DPadDown;

    throw runtime_error(fmt::format("Invalid controller button name \"{}\"", name));
  }

  bool Controller::has_button_named(const string& name)
  {
    // Dirty
    try {
      button_from_name(name);
    } catch (...) {
      return false;
    }

    return true;
  }

  string Controller::axis_name(Controller::Axis axis)
  {
    switch (axis) {
      case Controller::Axis::LeftX:
        return "LX";
      case Controller::Axis::LeftY:
        return "LY";
      case Controller::Axis::RightX:
        return "RX";
      case Controller::Axis::RightY:
        return "RY";
      case Controller::Axis::LeftTrigger:
        return "LT";
      case Controller::Axis::RightTrigger:
        return "RT";
    }

    return "UNK";
  }

  Controller::Axis Controller::axis_from_name(const string& name)
  {
    if (name == "LX")
      return Controller::Axis::LeftX;
    if (name == "LY")
      return Controller::Axis::LeftY;
    if (name == "RX")
      return Controller::Axis::RightX;
    if (name == "RY")
      return Controller::Axis::RightY;
    if (name == "LT")
      return Controller::Axis::LeftTrigger;
    if (name == "RT")
      return Controller::Axis::RightTrigger;

    throw runtime_error(fmt::format("Invalid controller axis name \"{}\"", name));
  }

  ostream& operator<<(ostream& os, Controller::Button btn)
  {
    os << Controller::button_name(btn);
    return os;
  }

  ostream& operator<<(ostream& os, Controller::Axis axis)
  {
    os << Controller::axis_name(axis);
    return os;
  }
}
