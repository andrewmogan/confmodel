cmake_minimum_required(VERSION 3.12)
project(confmodel VERSION 2.3.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(oksdalgen REQUIRED)
find_package(ers REQUIRED)
find_package(logging REQUIRED)
find_package(conffwk REQUIRED)
find_package(nlohmann_json REQUIRED)

#find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################

daq_oks_codegen(dunedaq.schema.xml NAMESPACE dunedaq::confmodel)

daq_add_library(dalMethods.cpp
  test_circular_dependency.cpp disabled-components.cpp
  LINK_LIBRARIES conffwk::conffwk
  logging::logging nlohmann_json::nlohmann_json)


daq_add_application(listApps list_apps.cxx
  LINK_LIBRARIES confmodel conffwk::conffwk)

daq_add_application(jsonable_test jsonable_test.cxx TEST
  LINK_LIBRARIES confmodel conffwk::conffwk logging::logging)
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings
daq_add_python_bindings(*.cpp LINK_LIBRARIES confmodel)


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

#daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

