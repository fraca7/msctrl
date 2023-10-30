
#include <stdexcept>
#include <algorithm>

#include <SDL2/SDL.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "SDLMain.h"

using namespace std;

namespace MSCtrl
{
  SDLMain::SDLMain()
    : m_controllers()
  {
    if (SDL_Init(SDL_INIT_GAMECONTROLLER|SDL_INIT_TIMER) < 0)
      throw runtime_error(fmt::format("Cannot initialize SDL: {}", SDL_GetError()));

    SDL_GameControllerEventState(SDL_ENABLE);
  }

  SDLMain::~SDLMain()
  {
    m_controllers.clear(); // Avoid double-free since SDL_Quit closes them
    SDL_Quit();
  }

  void SDLMain::loop()
  {
    SDL_Event evt;
    bool stop = false;
    while (!stop && SDL_WaitEvent(&evt)) {
      switch (evt.type) {
        case SDL_QUIT:
          spdlog::info("Quitting");
          stop = true;
          break;
        case SDL_CONTROLLERDEVICEADDED:
        {
          string name = SDL_GameControllerNameForIndex(evt.cdevice.which);

          spdlog::info("Controller {} added", name);

          if (on_controller_added(name)) {
            m_controllers.emplace_back(new Controller(evt.cdevice.which));
            on_controller_open(*m_controllers.back());

            spdlog::info("Controller {} opened", name);
          }
          break;
        }
        case SDL_CONTROLLERDEVICEREMOVED:
          m_controllers.erase(
            remove_if(m_controllers.begin(), m_controllers.end(), [&](const unique_ptr<Controller>& ctrl) {
              if (ctrl->matches(evt.cdevice.which)) {
                spdlog::info("Controller {} closed", ctrl->name());
                return true;
              }
              return false;
            }),
            m_controllers.end()
            );
          break;
        case SDL_CONTROLLERBUTTONUP:
          for (auto& ctrl : m_controllers) {
            if (ctrl->matches(evt.cbutton.which)) {
              ctrl->on_button_release(evt.cbutton.button);
              break;
            }
          }
          break;
        case SDL_CONTROLLERBUTTONDOWN:
          for (auto& ctrl : m_controllers) {
            if (ctrl->matches(evt.cbutton.which)) {
              ctrl->on_button_press(evt.cbutton.button);
              break;
            }
          }
          break;
        case SDL_CONTROLLERAXISMOTION:
          for (auto& ctrl : m_controllers) {
            if (ctrl->matches(evt.caxis.which)) {
              ctrl->on_axis_motion(evt.caxis.axis, evt.caxis.value);
              break;
            }
          }
          break;
        case SDL_CONTROLLERSENSORUPDATE:
          for (auto& ctrl : m_controllers) {
            if (ctrl->matches(evt.csensor.which)) {
              switch (evt.csensor.sensor) {
                case SDL_SENSOR_GYRO:
                  ctrl->on_gyro_update(evt.csensor.timestamp, evt.csensor.data[0], evt.csensor.data[1], evt.csensor.data[2]);
                  break;
                default:
                  break;
              }
              break;
            }
          }
          break;
        case SDL_JOYDEVICEADDED:
          if (!SDL_IsGameController(evt.jdevice.which)) {
            spdlog::warn("Joystick {} added, but is not a game controller", SDL_JoystickNameForIndex(evt.jdevice.which));
          }
          break;
        default:
          break;
      }
    }
  }
}
