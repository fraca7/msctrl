
#include <stdexcept>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "AxisMap.h"

using namespace std;

namespace MSCtrl
{
  AxisMap::AxisMap(MasterSystem& ms, Axis axis)
    : m_ms(ms),
      m_axis(axis),
      m_deadzone_lo(0.4f),
      m_deadzone_hi(0.5f),
      m_angle_hysteresis(5.0f),
      m_xval(0.0f),
      m_yval(0.0f),
      m_area(0)
  {
  }

  void AxisMap::set_deadzone(float lo, float hi)
  {
    if (lo < 0.0f)
      throw runtime_error(fmt::format("Value {} for low deadzone is negative", lo));
    if (lo >= 1.0f)
      throw runtime_error(fmt::format("Value {} for low deadzone is 1 or more", lo));
    if (hi < 0.0f)
      throw runtime_error(fmt::format("Value {} for high deadzone is negative", hi));
    if (hi >= 1.0f)
      throw runtime_error(fmt::format("Value {} for high deadzone is 1 or more", hi));
    if (lo >= hi)
      throw runtime_error(fmt::format("Value {} for high deadzone is less than value {} for low deadzone", hi, lo));

    m_deadzone_lo = lo;
    m_deadzone_hi = hi;
  }

  void AxisMap::set_angle_hysteresis(float delta)
  {
    if (delta < 0.0f)
      throw runtime_error(fmt::format("Value {} for angle hysteresis is negative", delta));
    if (delta >= 22.5f)
      throw runtime_error(fmt::format("Value {} for angle hysteresis is too large (>= 22.5)", delta));

    m_angle_hysteresis = delta;
  }

  void AxisMap::add_to(Controller& ctrl)
  {
    spdlog::info("Add {} axis mapping to {}; lo={:.2f}, hi={:.2f}, ht={:.2f}", (m_axis == Axis::Left) ? "left" : "right", ctrl.name(), m_deadzone_lo, m_deadzone_hi, m_angle_hysteresis);

    Controller::Listener::add_to(ctrl);
  }

  void AxisMap::on_axis_motion(Controller& ctrl, Controller::Axis axis, float value)
  {
    float curr_x = m_xval;
    float curr_y = m_yval;

    switch (m_axis) {
      case Axis::Left:
        switch (axis) {
          case Controller::Axis::LeftX:
            curr_x = value;
            break;
          case Controller::Axis::LeftY:
            curr_y = value;
            break;
          default:
            return;
        }
        break;
      case Axis::Right:
        switch (axis) {
          case Controller::Axis::RightX:
            curr_x = value;
            break;
          case Controller::Axis::RightY:
            curr_y = value;
            break;
          default:
            return;
        }
        break;
    }

    spdlog::trace("{} axis for {}: {:.2f}/{:.2f}", (m_axis == Axis::Left) ? "Left" : "Right", ctrl.name(), curr_x, curr_y);

    m_xval = curr_x;
    m_yval = curr_y;

    // 1. Hysteresis for in/out of deadzone state
    float dist = curr_x * curr_x + curr_y * curr_y;
    float angle = (atan2f(1.0f * curr_y, 1.0f * curr_x) + M_PI) * 180 / M_PI;
    int area = m_area;

    if ((m_area == 0) && (dist >= m_deadzone_hi * m_deadzone_hi)) {
      area = ((int)((angle + 22.5) / 45) % 8) + 1;
      spdlog::debug("{} axis for {} out of deadzone in area {}", (m_axis == Axis::Left) ? "Left" : "Right", ctrl.name(), area);
    } else if ((m_area != 0) && (dist <= m_deadzone_lo * m_deadzone_lo)) {
      area = 0;
      spdlog::debug("{} axis for {} in deadzone", (m_axis == Axis::Left) ? "Left" : "Right", ctrl.name());
    } else if (m_area != 0) {
      // Still out of deadzone. Consider the current area a little bigger, for a kind of angular hysteresis.
      bool is_in = false;
      switch (m_area) {
        case 1:
          if ((angle >= 337.5 - m_angle_hysteresis) || (angle <= 22.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 2:
          if ((angle >= 22.5 - m_angle_hysteresis) && (angle <= 67.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 3:
          if ((angle >= 67.5 - m_angle_hysteresis) && (angle <= 112.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 4:
          if ((angle >= 112.5 - m_angle_hysteresis) && (angle <= 157.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 5:
          if ((angle >= 157.5 - m_angle_hysteresis) && (angle <= 202.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 6:
          if ((angle >= 202.5 - m_angle_hysteresis) && (angle <= 247.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 7:
          if ((angle >= 247.5 - m_angle_hysteresis) && (angle <= 292.5 + m_angle_hysteresis))
            is_in = true;
          break;
        case 8:
          if ((angle >= 292.5 - m_angle_hysteresis) && (angle <= 337.5 + m_angle_hysteresis))
            is_in = true;
          break;
      }

      if (!is_in) {
        area = ((int)((angle + 22.5) / 45) % 8) + 1;
        spdlog::debug("{} axis for {} now in area {} (angle={:.2f}", (m_axis == Axis::Left) ? "Left" : "Right", ctrl.name(), area, angle);
      }
    } else {
      // Still in deadzone
      area = 0;
    }

    uint8_t prev_state = get_state(m_area);
    m_area = area;
    uint8_t curr_state = get_state(m_area);

    uint8_t diff = prev_state ^ curr_state;

    if ((diff & SDL_HAT_LEFT) != 0)
      m_ms.set_button_state(MasterSystem::Button::Left, (curr_state & SDL_HAT_LEFT) != 0);
    if ((diff & SDL_HAT_RIGHT) != 0)
      m_ms.set_button_state(MasterSystem::Button::Right, (curr_state & SDL_HAT_RIGHT) != 0);
    if ((diff & SDL_HAT_UP) != 0)
      m_ms.set_button_state(MasterSystem::Button::Up, (curr_state & SDL_HAT_UP) != 0);
    if ((diff & SDL_HAT_DOWN) != 0)
      m_ms.set_button_state(MasterSystem::Button::Down, (curr_state & SDL_HAT_DOWN) != 0);
  }

  uint8_t AxisMap::get_state(int area) {
    switch (area) {
      case 1:
        return SDL_HAT_LEFT;
      case 2:
        return SDL_HAT_LEFTUP;
      case 3:
        return SDL_HAT_UP;
      case 4:
        return SDL_HAT_RIGHTUP;
      case 5:
        return SDL_HAT_RIGHT;
      case 6:
        return SDL_HAT_RIGHTDOWN;
      case 7:
        return SDL_HAT_DOWN;
      case 8:
        return SDL_HAT_LEFTDOWN;
      default:
        break;
    }

    return 0;
  }

  string AxisMap::axis_name(Axis axis)
  {
    switch (axis) {
      case Axis::Left:
        return "L";
      case Axis::Right:
        return "R";
    }

    return "UNK";
  }

  AxisMap::Axis AxisMap::axis_from_name(const string& name)
  {
    if (name == "L")
      return Axis::Left;
    if (name == "R")
      return Axis::Right;

    throw runtime_error(fmt::format("Invalid AxisMap axis name \"{}\"", name));
  }
}
