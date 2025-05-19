cmake_minimum_required(VERSION 3.12)
project(confmodel VERSION 2.2.1)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(okssystem REQUIRED)
find_package(oksdalgen REQUIRED)
find_package(ers REQUIRED)
find_package(logging REQUIRED)
find_package(conffwk REQUIRED)
find_package(nlohmann_json REQUIRED)

#find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################

daq_add_dal_library(dunedaq.schema.xml SOURCES dalMethods.cpp test_circular_dependency.cpp disabled-components.cpp LINK_LIBRARIES okssystem::okssystem logging::logging nlohmann_json::nlohmann_json)

daq_add_application(listApps list_apps.cxx
  LINK_LIBRARIES confmodel_dal conffwk::conffwk)

daq_add_application(disable_test disable_test.cxx TEST
  LINK_LIBRARIES confmodel_dal conffwk::conffwk logging::logging)

daq_add_application(jsonable_test jsonable_test.cxx TEST
  LINK_LIBRARIES confmodel_dal conffwk::conffwk logging::logging)
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_python_bindings
daq_add_python_bindings(*.cpp LINK_LIBRARIES confmodel_dal)


##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

#daq_add_unit_test(Placeholder_test LINK_LIBRARIES)  # Placeholder_test should be replaced with real unit tests

##############################################################################

daq_install()

