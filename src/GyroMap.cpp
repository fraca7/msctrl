
#include <stdexcept>
#include <regex>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "GyroMap.h"

using namespace std;

namespace MSCtrl
{
  GyroMap::GyroMap(MasterSystem& ms, GyroMap::Axis axis, MasterSystem::Button btn)
    : m_ms(ms),
      m_axis(axis),
      m_button(btn),
      m_button_state(false),
      m_threshold(20.0f * M_PI / 180),
      m_angle_delta(3.0f * M_PI / 180),
      m_IMU(GyroMap::axis_name(axis)),
      m_trigger_buttons(),
      m_pressed_buttons()
  {
  }

  void GyroMap::add_trigger_button(Controller::Button btn)
  {
    m_trigger_buttons.insert(btn);
  }

  void GyroMap::set_angle_threshold(float threshold)
  {
    m_threshold = threshold * M_PI / 180;
  }

  void GyroMap::set_angle_delta(float delta)
  {
    if (delta < 0)
      throw runtime_error(fmt::format("Angle delta {} is negative", delta));

    m_angle_delta = delta * M_PI / 180;
  }

  void GyroMap::add_to(Controller& ctrl)
  {
    spdlog::info("Add gyro mapping to {}; axis={}, threshold={:.2f}, hysteresis={:.2f}", ctrl.name(), GyroMap::axis_name(m_axis), m_threshold, m_angle_delta);

    Controller::Listener::add_to(ctrl);
  }

  void GyroMap::on_button_state(Controller& ctrl, Controller::Button btn, bool state)
  {
    if (m_trigger_buttons.size() == 0)
      return;

    unsigned prev_count = m_pressed_buttons.size();

    if (m_trigger_buttons.find(btn) != m_trigger_buttons.end()) {
      if (state) {
        m_pressed_buttons.insert(btn);
      } else {
        auto pos = m_pressed_buttons.find(btn);
        if (pos != m_pressed_buttons.end())
          m_pressed_buttons.erase(pos);
      }
    }

    unsigned curr_count = m_pressed_buttons.size();

    if ((prev_count != curr_count) && (curr_count == m_trigger_buttons.size())) {
      spdlog::info("Enable gyro on {} ({})", ctrl.name(), axis_name(m_axis));
      m_IMU.reset();
    } else if ((prev_count != curr_count) && (prev_count == m_trigger_buttons.size())) {
      spdlog::info("Disable gyro on {} ({})", ctrl.name(), axis_name(m_axis));
      if (m_button_state) {
        spdlog::info("Gyro release of {} on {} ({}) (disabled)", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis));
        m_ms.set_button_state(m_button, false);
        m_button_state = false;
      }
    }
  }

  void GyroMap::on_gyro_update(Controller& ctrl, uint32_t timestamp, float dx, float dy, float dz)
  {
    // Always update IMU
    switch (m_axis) {
      case Axis::PosX:
      case Axis::NegX:
        if (!m_IMU.update(ctrl, timestamp, dx))
          return;
        break;
      case Axis::PosY:
      case Axis::NegY:
        if (!m_IMU.update(ctrl, timestamp, dy))
          return;
        break;
      case Axis::PosZ:
      case Axis::NegZ:
        if (!m_IMU.update(ctrl, timestamp, dz))
          return;
        break;
    }

    if ((m_trigger_buttons.size() != 0) && (m_trigger_buttons.size() != m_pressed_buttons.size()))
      return;

    switch (m_axis) {
      case Axis::PosX:
      case Axis::NegX:
        if (m_button_state && (((m_axis == Axis::PosX) && (m_IMU.value() <= m_threshold - m_angle_delta)) || ((m_axis == Axis::NegX) && (m_IMU.value() >= -m_threshold + m_angle_delta)))) {
          spdlog::debug("Gyro release of {} on {} ({}) at {:.2f}", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis), m_IMU.value());
          m_ms.set_button_state(m_button, false);
          m_button_state = false;
        } else if (!m_button_state && (((m_axis == Axis::PosX) && (m_IMU.value() >= m_threshold)) || ((m_axis == Axis::NegX) && (m_IMU.value() <= -m_threshold)))) {
          spdlog::debug("Gyro press of {} on {} ({}) at {:.2f}", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis), m_IMU.value());
          m_ms.set_button_state(m_button, true);
          m_button_state = true;
        }
        break;
      case Axis::PosY:
      case Axis::NegY:
        if (m_button_state && (((m_axis == Axis::PosY) && (m_IMU.value() <= m_threshold - m_angle_delta)) || ((m_axis == Axis::NegY) && (m_IMU.value() >= -m_threshold + m_angle_delta)))) {
          spdlog::debug("Gyro release of {} on {} ({}) at {:.2f}", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis), m_IMU.value());
          m_ms.set_button_state(m_button, false);
          m_button_state = false;
        } else if (!m_button_state && (((m_axis == Axis::PosY) && (m_IMU.value() >= m_threshold)) || ((m_axis == Axis::NegY) && (m_IMU.value() <= -m_threshold)))) {
          spdlog::debug("Gyro press of {} on {} ({}) at {:.2f}", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis), m_IMU.value());
          m_ms.set_button_state(m_button, true);
          m_button_state = true;
        }
        break;
      case Axis::PosZ:
      case Axis::NegZ:
        if (m_button_state && (((m_axis == Axis::PosZ) && (m_IMU.value() <= m_threshold - m_angle_delta)) || ((m_axis == Axis::NegZ) && (m_IMU.value() >= -m_threshold + m_angle_delta)))) {
          spdlog::debug("Gyro release of {} on {} ({}) at {:.2f}", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis), m_IMU.value());
          m_ms.set_button_state(m_button, false);
          m_button_state = false;
        } else if (!m_button_state && (((m_axis == Axis::PosZ) && (m_IMU.value() >= m_threshold)) || ((m_axis == Axis::NegZ) && (m_IMU.value() <= -m_threshold)))) {
          spdlog::debug("Gyro press of {} on {} ({}) at {:.2f}", MasterSystem::button_name(m_button), ctrl.name(), axis_name(m_axis), m_IMU.value());
          m_ms.set_button_state(m_button, true);
          m_button_state = true;
        }
        break;
    }
  }

  string GyroMap::axis_name(GyroMap::Axis axis)
  {
    switch (axis) {
      case Axis::PosX:
        return "+X";
      case Axis::NegX:
        return "-X";
      case Axis::PosY:
        return "+Y";
      case Axis::NegY:
        return "-Y";
      case Axis::PosZ:
        return "+Z";
      case Axis::NegZ:
        return "-Z";
    }

    return "UNK";
  }

  GyroMap::Axis GyroMap::axis_from_name(const string& name)
  {
    regex rx(R"(((?:-|\+)?)([XYZ]))");
    smatch mt;
    if (!regex_match(name, mt, rx))
      throw runtime_error(fmt::format("Invalid gyro axis \"{}\"", name));

    if (mt[2].str() == "X")
      return (mt[1].str() == "-") ? Axis::NegX : Axis::PosX;
    if (mt[2].str() == "Y")
      return (mt[1].str() == "-") ? Axis::NegY : Axis::PosY;
    if (mt[2].str() == "Z")
      return (mt[1].str() == "-") ? Axis::NegZ : Axis::PosZ;

    throw runtime_error(fmt::format("Invalid axis name \"{}\"", name));
  }
}
