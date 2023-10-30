
#ifndef _MSCTRL_CONTROLLER_H
#define _MSCTRL_CONTROLLER_H

#include <ostream>
#include <string>
#include <list>

#include <SDL2/SDL.h>

namespace MSCtrl
{
  class Controller
  {
  public:
    enum class Button {
      A,
      B,
      X,
      Y,
      LeftShoulder,
      RightShoulder,
      LeftTrigger,
      RightTrigger,
      DPadLeft,
      DPadRight,
      DPadUp,
      DPadDown
    };

    enum class Axis {
      LeftX,
      LeftY,
      RightX,
      RightY,
      LeftTrigger,
      RightTrigger
    };

    class Listener
    {
    public:
      virtual void on_button_state(Controller&, Button, bool) {};
      virtual void on_axis_motion(Controller&, Axis, float) {};
      virtual void on_gyro_update(Controller&, uint32_t, float, float, float) {};

      virtual void add_to(Controller&);
    };

    ~Controller();

    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;

    /**
     * Set the threshold to consider triggers as pressed
     * @param value Threshold value (0.0 is fully released, 1.0 is fully pressed) default is 0.5
     */
    void set_trigger_threshold(float value);

    void add_listener(Listener*);
    void remove_listener(Listener*);

    std::string name() const;

    static std::string button_name(Button);
    static Button button_from_name(const std::string&);
    static bool has_button_named(const std::string&);

    static std::string axis_name(Axis);
    static Axis axis_from_name(const std::string&);

  private:
    SDL_GameController* m_handle;
    float m_trigger_threshold;
    std::list<Listener*> m_listeners;
    float m_last_left;
    float m_last_right;

    friend class SDLMain;

    Controller(int);

    bool matches(SDL_JoystickID) const;

    void on_button_press(uint8_t);
    void on_button_release(uint8_t);
    void on_axis_motion(uint8_t, int16_t);
    void on_gyro_update(uint32_t, float, float, float);

    bool map_button(uint8_t, Button&);
    bool map_axis(uint8_t, Axis&);
  };

  std::ostream& operator<<(std::ostream&, Controller::Button);
  std::ostream& operator<<(std::ostream&, Controller::Axis);
}

#endif /* _MSCTRL_CONTROLLER_H */
