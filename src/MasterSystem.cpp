
#include <iostream>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#ifdef ENABLE_GPIO
#include <pigpio.h>
#endif

#include "MasterSystem.h"

using namespace std;

namespace MSCtrl
{
  MasterSystem::MasterSystem()
#ifdef ENABLE_GPIO
    : m_map()
#endif
  {
#ifdef ENABLE_GPIO
    gpioCfgSetInternals(gpioCfgGetInternals() | PI_CFG_NOSIGHANDLER);
    if (gpioInitialise() < 0)
      throw runtime_error("Cannot initialize GPIO");

    spdlog::info("GPIO initialized");

    // Defaults (my own setup)
    set_gpio_map(Button::B1, 27U);
    set_gpio_map(Button::B2, 22U);
    set_gpio_map(Button::Up, 2U);
    set_gpio_map(Button::Down, 3U);
    set_gpio_map(Button::Left, 4U);
    set_gpio_map(Button::Right, 17U);
#else
    spdlog::info("GPIO disabled");
#endif
  }

  MasterSystem::~MasterSystem()
  {
#ifdef ENABLE_GPIO
    gpioTerminate();
#endif
  }

#ifdef ENABLE_GPIO
  void MasterSystem::set_gpio_map(Button btn, unsigned port)
  {
    m_map.insert(make_pair(btn, port));

    int status;
    if ((status = gpioSetMode(port, PI_OUTPUT)) != 0) {
      switch (status) {
        case PI_BAD_GPIO:
          throw runtime_error(fmt::format("Bad GPIO port {}", port));
        case PI_BAD_MODE:
          throw runtime_error(fmt::format("Bad mode for GPIO {}", port));
        default:
          throw runtime_error(fmt::format("Unknown error initializing GPIO {}: {}", port, status));
      }
    }
  }
#endif

  void MasterSystem::set_button_state(Button btn, bool state)
  {
#ifdef ENABLE_GPIO
    unsigned port = m_map[btn];

    spdlog::debug("Set GPIO {} to {}", port, state ? "HI" : "LO");

    int status;
    if ((status = gpioWrite(port, state ? 1 : 0)) != 0) {
      switch (status) {
        case PI_BAD_GPIO:
          throw runtime_error(fmt::format("Bad GPIO port {}", port));
        case PI_BAD_LEVEL:
          throw runtime_error(fmt::format("Bad level for GPIO {}", port));
        default:
          throw runtime_error(fmt::format("Unknown error setting state {} for GPIO {}: {}", state, port, status));
      }
    }
#else
    spdlog::info("{} button {}", state ? "Press" : "Release", MasterSystem::button_name(btn));
#endif
  }

  string MasterSystem::button_name(MasterSystem::Button btn)
  {
    switch (btn) {
      case Button::B1:
        return "B1";
      case Button::B2:
        return "B2";
      case Button::Up:
        return "Up";
      case Button::Down:
        return "Down";
      case Button::Left:
        return "Left";
      case Button::Right:
        return "Right";
    }

    return "Unknown";
  }

  MasterSystem::Button MasterSystem::button_from_name(const string& name)
  {
    if (name == "B1")
      return Button::B1;
    if (name == "B2")
      return Button::B2;
    if (name == "U")
      return Button::Up;
    if (name == "D")
      return Button::Down;
    if (name == "L")
      return Button::Left;
    if (name == "R")
      return Button::Right;

    throw runtime_error(fmt::format("Invalid MS button name \"{}\"", name));
  }
}
