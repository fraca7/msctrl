
#ifndef _MSCTRL_AXISMAP_H
#define _MSCTRL_AXISMAP_H

#include <src/MasterSystem.h>
#include <src/Controller.h>

namespace MSCtrl
{
  class AxisMap : public Controller::Listener
  {
  public:
    enum class Axis {
      Left,
      Right
    };

    AxisMap(MasterSystem&, Axis);

    /**
     * Set deadzone thresholds
     * @param lo Low threshold (default 0.4)
     * @param hi High threshold (default 0.5)
     */
    void set_deadzone(float lo, float hi);

    /**
     * Set delta for angle hysteresis (to avoid rapid press/release of
     * buttons when the stick is near the separation of two areas)
     * @param delta Angle delta in degrees (default 5)
     */
    void set_angle_hysteresis(float delta);

    void add_to(Controller&) override;
    void on_axis_motion(Controller&, Controller::Axis, float) override;

    static std::string axis_name(Axis);
    static Axis axis_from_name(const std::string&);

  private:
    MasterSystem& m_ms;
    Axis m_axis;
    float m_deadzone_lo;
    float m_deadzone_hi;
    float m_angle_hysteresis;
    float m_xval;
    float m_yval;
    int m_area;

    uint8_t get_state(int);
  };
}

#endif /* _MSCTRL_AXISMAP_H */
