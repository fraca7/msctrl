
cmake_minimum_required(VERSION 3.8)

project(MSCtrl
  VERSION 1.0
  LANGUAGES CXX
  )

find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED
  HINTS /opt/sdl)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

find_path(pigpio_INCLUDE_DIR
  NAMES pigpio.h)
find_library(pigpio_LIBRARY
  NAMES libpigpio.so)

if (pigpio_INCLUDE_DIR)
  set(ENABLE_GPIO ON)
  include_directories("${pigpio_INCLUDE_DIR}")
  add_definitions(-DENABLE_GPIO)
else ()
  set(ENABLE_GPIO OFF)
  message(WARNING "Cannot find pigpio include path; using stdout stub")
endif ()

include_directories("${SDL2_INCLUDE_DIRS}" "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

option(ENABLE_GYRO_CALIBRATION "Calibrate on the first values of gyro readings" ON)

configure_file(
  src/Configure.h.in
  src/Configure.h
  )

add_executable(msctrl
  src/msctrl.cpp
  src/Configure.h
  src/utils.h
  src/utils.cpp
  src/CLParser.h
  src/CLParser.cpp
  src/SDLMain.h
  src/SDLMain.cpp
  src/Controller.h
  src/Controller.cpp
  src/ButtonMap.h
  src/ButtonMap.cpp
  src/HatMap.h
  src/HatMap.cpp
  src/IMUIntegrator.h
  src/IMUIntegrator.cpp
  src/GyroMap.h
  src/GyroMap.cpp
  src/AxisMap.h
  src/AxisMap.cpp
  src/MasterSystem.h
  src/MasterSystem.cpp
  )
target_link_libraries(msctrl fmt::fmt ${SDL2_LIBRARIES} nlohmann_json::nlohmann_json spdlog::spdlog)
if (ENABLE_GPIO)
  target_link_libraries(msctrl ${pigpio_LIBRARY})
endif ()

target_compile_options(msctrl PRIVATE -Wall)
