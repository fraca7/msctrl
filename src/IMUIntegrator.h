
#ifndef _MSCTRL_IMUINTEGRATOR_H
#define _MSCTRL_IMUINTEGRATOR_H

#include <string>

#include <src/Configure.h>

namespace MSCtrl
{
  class Controller;

  class IMUIntegrator
  {
  public:
    IMUIntegrator(const std::string&);

    bool update(Controller&, uint32_t, float);

    float value() const {
      return m_value;
    }

    void reset() {
      m_value = 0.0f;
    }

  private:
    std::string m_name;
    uint32_t m_last_timestamp;
    float m_value;
    float m_last_value;
#ifdef ENABLE_GYRO_CALIBRATION
    float m_calib_value;
    unsigned m_count;
#endif
    enum class State {
#ifdef ENABLE_GYRO_CALIBRATION
      Calibrating,
#else
      Starting,
#endif
      Running
    };
    State m_state;
  };
}

#endif /* _MSCTRL_IMUINTEGRATOR_H */
