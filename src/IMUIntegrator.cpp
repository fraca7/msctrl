
#include <spdlog/spdlog.h>

#include "IMUIntegrator.h"
#include "Controller.h"

using namespace std;

namespace MSCtrl
{
  IMUIntegrator::IMUIntegrator(const string& name)
    : m_name(name),
      m_last_timestamp(0),
      m_value(0.0f),
      m_last_value(0.0f),
#ifdef ENABLE_GYRO_CALIBRATION
      m_calib_value(0.0f),
      m_count(0),
      m_state(State::Calibrating)
#else
      m_state(State::Starting)
#endif
  {
  }

  bool IMUIntegrator::update(Controller& ctrl, uint32_t timestamp, float value)
  {
    switch (m_state) {
#ifdef ENABLE_GYRO_CALIBRATION
      case State::Calibrating:
        if (m_count == 0)
          spdlog::info("Starting calibration on {} ({})", ctrl.name(), m_name);

        m_calib_value += value;
        if (++m_count == 80) {
          m_calib_value /= m_count;

          spdlog::info("{} ({}) calibrated: {:.3f}", ctrl.name(), m_name, m_calib_value);

          m_last_timestamp = timestamp;
          m_last_value = value;

          m_state = State::Running;
        }
        break;
#else
      case State::Starting:
        m_last_timestamp = timestamp;
        m_last_value = value;
        m_state = State::Running;
        break;
#endif
      case State::Running:
      {
#ifdef ENABLE_GYRO_CALIBRATION
        value -= m_calib_value;
#endif

        uint32_t dt = timestamp - m_last_timestamp;

        m_value += (m_last_value + value) / 2 * dt / 1000;

        m_last_value = value;
        m_last_timestamp = timestamp;

        return true;
      }
    }

    return false;
  }
}
