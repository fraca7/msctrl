
#ifndef _MSCTRL_GYROMAP_H
#define _MSCTRL_GYROMAP_H

#include <set>

#include <src/Configure.h>
#include <src/MasterSystem.h>
#include <src/Controller.h>
#include <src/IMUIntegrator.h>

namespace MSCtrl
{
  class GyroMap : public Controller::Listener
  {
  public:
    enum class Axis {
      PosX,
      NegX,
      PosY,
      NegY,
      PosZ,
      NegZ
    };

    GyroMap(MasterSystem&, Axis, MasterSystem::Button);

    /**
     * Add a trigger button.
     */
    void add_trigger_button(Controller::Button);

    /**
     * Set the angle threshold in degrees
     */
    void set_angle_threshold(float);

    /**
     * Set the angle hysteresis delta in degrees (to avoid repeated press/release when near the threshold)
     */
    void set_angle_delta(float delta);

    void add_to(Controller&) override;
    void on_button_state(Controller&, Controller::Button, bool) override;
    void on_gyro_update(Controller&, uint32_t, float, float, float) override;

    static std::string axis_name(Axis);
    static Axis axis_from_name(const std::string&);

  private:
    MasterSystem& m_ms;
    Axis m_axis;
    MasterSystem::Button m_button;
    bool m_button_state;
    float m_threshold;
    float m_angle_delta;

    IMUIntegrator m_IMU;

    std::set<Controller::Button> m_trigger_buttons;
    std::set<Controller::Button> m_pressed_buttons;
  };
}

#endif /* _MSCTRL_GYROMAP_H */
