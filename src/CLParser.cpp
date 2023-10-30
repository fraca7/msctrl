
#include <iostream>
#include <regex>
#include <memory>
#include <map>
#include <functional>
#include <fstream>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "CLParser.h"
#include "ButtonMap.h"
#include "HatMap.h"
#include "AxisMap.h"
#include "GyroMap.h"
#include "utils.h"

using namespace std;

namespace MSCtrl
{
  CLParser::CLParser()
  {
  }

  void CLParser::parse(MasterSystem& ms, ConfigurationTarget& target, int argc, char* argv[])
  {
    unique_ptr<ButtonMap> b1(new ButtonMap(ms, ButtonMap::Button::B1));
    unique_ptr<ButtonMap> b2(new ButtonMap(ms, ButtonMap::Button::B2));

    nlohmann::json json_config;
    json_config["version"] = 1;
    json_config["config"] = nlohmann::json();
    json_config["config"]["buttons"] = nlohmann::json();
    json_config["config"]["sticks"] = nlohmann::json::array();
    json_config["config"]["gyro"] = nlohmann::json::array();
    string config_filename = "";

    int state = 0;
    for (int i = 1; i < argc; ++i) {
      switch (state) {
        case 0:
          if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
            throw usage_exception();
          else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--button"))
            state = 1;
          else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--trigger"))
            state = 2;
          else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dpad")) {
            target.add_map(new HatMap(ms));
            json_config["config"]["hat"] = true;
          } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--stick"))
            state = 3;
          else if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--gyro"))
            state = 4;
          else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output"))
            state = 5;
          else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--config"))
            state = 6;
          else
            throw runtime_error(fmt::format("Unrecognized argument \"{}\"", argv[i]));
          break;
        case 1:
        {
          regex rx(R"((A|B|X|Y|LS|RS|LT|RT):(B1|B2))");
          smatch mt;
          string v(argv[i]);
          if (!regex_match(v, mt, rx))
            throw runtime_error(fmt::format("Invalid mapping specification \"{}\"", v));

          Controller::Button src = Controller::button_from_name(mt[1].str());
          if (mt[2].str() == "B1")
            b1->add_source_button(src);
          if (mt[2].str() == "B2")
            b2->add_source_button(src);

          json_config["config"]["buttons"][mt[1].str()] = mt[2].str();

          state = 0;
          break;
        }
        case 2:
          json_config["config"]["trigger_threshold"] = atof(argv[i]);
          target.set_trigger_threshold(atof(argv[i]));
          state = 0;
          break;
        case 3:
        {
          unique_ptr<AxisMap> map(nullptr);
          float hi = 0.5f, lo = 0.4f;

          nlohmann::json config;

          split_string(argv[i], ',', [&](int index, const string& part) {
            if (index == 0) {
              config["which"] = part;
              map.reset(new AxisMap(ms, AxisMap::axis_from_name(part)));
            } else {
              regex rx(R"((lo|hi|ht)=(\d+(?:\.\d+)?))");
              smatch mt;
              if (!regex_match(part, mt, rx))
                throw runtime_error(fmt::format("Invalid parameter for stick: \"{}\"", part));

              float value = stof(mt[2].str());

              if (mt[1].str() == "lo") {
                lo = value;
                config["lo"] = value;
              } else if (mt[1].str() == "hi") {
                hi = value;
                config["hi"] = value;
              } else {
                map->set_angle_hysteresis(value);
                config["ht"] = value;
              }
            }
          });

          if (!map)
            throw runtime_error("Empty stick specification");

          map->set_deadzone(lo, hi);
          target.add_map(map.release());
          json_config["config"]["sticks"].push_back(config);

          state = 0;
          break;
        }
        case 4:
        {
          nlohmann::json config;
          config["triggers"] = nlohmann::json::array();

          unique_ptr<GyroMap> map(nullptr);

          split_string(argv[i], ',', [&](unsigned index, const string& part) {
            if (index == 0) {
              regex rx(R"(((?:\+|-)?[XYZ]):([UDLR]))");
              smatch mt;
              if (!regex_match(part, mt, rx))
                throw runtime_error(fmt::format("Invalid gyro specification \"{}\"", part));

              config["axis"] = mt[1].str();
              config["button"] = mt[2].str();

              map.reset(new GyroMap(ms, GyroMap::axis_from_name(mt[1].str()), MasterSystem::button_from_name(mt[2].str())));
            } else {
              if (Controller::has_button_named(part)) {
                map->add_trigger_button(Controller::button_from_name(part));
                config["triggers"].push_back(part);
                return;
              }

              regex rx(R"((th|hy)=(\d+(?:\.\d+)?))");
              smatch mt;
              if (!regex_match(part, mt, rx))
                throw runtime_error(fmt::format("Invalid gyro option \"{}\"", part));

              float value = stof(mt[2].str());

              if (mt[1].str() == "th") {
                map->set_angle_threshold(value);
                config["threshold"] = value;
              }
              if (mt[2].str() == "hy") {
                map->set_angle_delta(value);
                config["delta"] = value;
              }
            }
          });

          target.add_map(map.release());
          json_config["config"]["gyro"].push_back(config);

          state = 0;
          break;
        }
        case 5:
          config_filename = argv[i];
          state = 0;
          break;
        case 6:
        {
          ifstream ifs(argv[i]);
          auto data = nlohmann::json::parse(ifs);

          // There's only version 1 for now, don't check

          for (nlohmann::json::iterator pos = data["config"]["buttons"].begin(); pos != data["config"]["buttons"].end(); ++pos) {
            auto src = Controller::button_from_name(pos.key());
            
            if (pos.value() == "B1")
              b1->add_source_button(src);
            if (pos.value() == "B2")
              b2->add_source_button(src);
          }

          for (const auto& stick : data["config"]["sticks"]) {
            unique_ptr<AxisMap> map(new AxisMap(ms, AxisMap::axis_from_name(stick["which"])));
            map->set_deadzone(stick["lo"], stick["hi"]);
            if (stick.contains("ht"))
              map->set_angle_hysteresis(stick["ht"]);

            target.add_map(map.release());
          }

          for (const auto& gyro : data["config"]["gyro"]) {
            unique_ptr<GyroMap> map(new GyroMap(ms, GyroMap::axis_from_name(gyro["axis"]), MasterSystem::button_from_name(gyro["button"])));

            if (gyro.contains("threshold"))
              map->set_angle_threshold(gyro["threshold"]);
            if (gyro.contains("delta"))
              map->set_angle_delta(gyro["delta"]);

            for (const auto& name : gyro["triggers"])
              map->add_trigger_button(Controller::button_from_name(name));

            target.add_map(map.release());
          }

          if (data["config"].contains("hat"))
            target.add_map(new HatMap(ms));

          if (data["config"].contains("trigger_threshold")) {
            target.set_trigger_threshold(data["config"]["trigger_threshold"]);
          }

          state = 0;
          break;
        }
      }
    }

    switch (state) {
      case 0:
        break;
      case 1:
        throw runtime_error("-b/--button without value");
      case 2:
        throw runtime_error("-t/--trigger without value");
      case 3:
        throw runtime_error("-s/--stick without value");
      case 4:
        throw runtime_error("-g/--gyro without value");
      case 5:
        throw runtime_error("-o/--output without filename");
      case 6:
        throw runtime_error("-c/--config without filename");
    }

    target.add_map(b1.release());
    target.add_map(b2.release());

    if (config_filename != "") {
      ofstream ofs(config_filename);
      ofs << json_config.dump(2);
    }
  }

  void CLParser::usage()
  {
    cerr << "Usage: msctrl [options]" << endl;
    cerr << "Options:" << endl;
    cerr << "  -h, --help             Display this and exit" << endl;

    cerr << "  -b, --button <map>     Map a button. <map> is of the form <src>:<dst> where" << endl;
    cerr << "                            <src> is a controller button: A,B,X,Y,LS,RS,LT,RT (left/right shoulder/trigger)" << endl;
    cerr << "                            <dst> is a Master System button (B1 or B2)" << endl;
    cerr << "                         This option can be repeated." << endl;

    cerr << "  -t, --trigger <val>    Sets the threshold to consider left/right triggers as buttons (default 0.5)" << endl;

    cerr << "  -d, --dpad             Map the controller digital pad to the Master System's pad" << endl;

    cerr << "  -s, --stick <spec>     Map a stick to the Master System's pad. <spec> is of the form <stick>[,<params>] where" << endl;
    cerr << "                         <stick> is either L (left) or R (right), and <params> is a comma-separated list of" << endl;
    cerr << "                         configuration values of the form <k>=<v>. Values for <k> may be" << endl;
    cerr << "                            lo: Low deadzone threshold (0 to 1, default 0.4)" << endl;
    cerr << "                            hi: High deadzone threshold (0 to 1, default 0.5)" << endl;
    cerr << "                            ht: Angle hysteresis threshold in degrees (default 5)" << endl;
    cerr << "                         Example: -s L,lo=0.2,hi=0.3" << endl;

    cerr << "  -g, --gyro <spec>      Map a gyro axis to a dpad button. <spec> is of the form <axis>:<button>[,options...]" << endl;
    cerr << "                         <axis> may be X, -X, Y, -Y, Z or -Z. <button> can be U,D,L or R (for up, down, left, right)." << endl;
    cerr << "                         Options is a comma-separated list of either button names (X,Y,A,etc) or <k>=<v>, where <k> may be" << endl;
    cerr << "                            th: Angle threshold in degrees (default 20)" << endl;
    cerr << "                            hy: Hysteresis angle threshold in degrees (default 3)" << endl;
    cerr << "                         If no buttons are specified, the mapping will always be enabled. Else, it will only be enabled" << endl;
    cerr << "                         whell all those buttons are pressed, and the neutral state is the controller's position when" << endl;
    cerr << "                         they were pressed." << endl;
    cerr << "                         Example: -g -X,L,th=15,LS,RS" << endl;

    cerr << "  -o, --output <name>    Save configuration as JSON to the specified file" << endl;

    cerr << "  -c, --config           Load specified JSON file before proceeding" << endl;
  }
}
