
#include <iostream>
#include <thread>
#include <chrono>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "SDLMain.h"
#include "CLParser.h"

using namespace std;
using namespace MSCtrl;

class Dispatcher : public SDLMain, public CLParser, public CLParser::ConfigurationTarget
{
public:
  Dispatcher(int argc, char* argv[])
    : m_ms(),
      m_trigger_threshold(0.5f),
      m_remappings() {
    try {
      parse(m_ms, *this, argc, argv);
    } catch (const exception&) {
      usage();
      throw;
    }
  }

  bool on_controller_added(const string& name) override {
    return true;
  }

  void on_controller_open(Controller& ctrl) override {
    for (auto& ptr : m_remappings)
      ptr->add_to(ctrl);
    ctrl.set_trigger_threshold(m_trigger_threshold);
  }

  void add_map(Controller::Listener* map) override {
    m_remappings.emplace_back(map);
  }

  void set_trigger_threshold(float value) override {
    spdlog::debug("Global trigger threshold: {}", value);
    m_trigger_threshold = value;
  }

private:
  MasterSystem m_ms;
  float m_trigger_threshold;
  list<unique_ptr<Controller::Listener>> m_remappings;
};

int main(int argc, char* argv[]) {
  spdlog::set_level(spdlog::level::debug);

  try {
    Dispatcher sdl(argc, argv);

    sdl.loop();
  } catch (const CLParser::usage_exception&) {
    // Nothing
  } catch (const exception& exc) {
    cerr << "Error: " << exc.what() << endl;
    return 1;
  }

  return 0;
}
