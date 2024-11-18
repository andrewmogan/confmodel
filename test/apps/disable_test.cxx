#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Component.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include <iostream>
#include <string>

using namespace dunedaq;

void listApps(const confmodel::Session* session) {
  for (auto app : session->get_all_applications()) {
    std::cout << "Application: " << app->UID();
    auto res = app->cast<confmodel::ResourceSet>();
    if (res) {
      if (res->disabled(*session)) {
        std::cout << "<disabled>";
      }
      else {
        for (auto mod : res->get_contains()) {
          std::cout << " " << mod->UID();
          if (mod->disabled(*session)) {
            std::cout << "<disabled>";
          }
        }
      }
    }
    auto daqApp = app->cast<confmodel::DaqApplication>();
    if (daqApp) {
      std::cout << " Modules:";
      for (auto mod : daqApp->get_modules()) {
        std::cout << " " << mod->UID();
      }
    }
    std::cout << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " session database-file\n";
    return 0;
  }

  std::string confimpl = "oksconflibs:" + std::string(argv[2]);
  auto confdb = new conffwk::Configuration(confimpl);

  std::string sessionName(argv[1]);

  
  dunedaq::logging::Logging::setup(sessionName, "disable-test");

  auto session = confdb->get<confmodel::Session>(sessionName);
  if (session == nullptr) {
    std::cerr << "Session " << sessionName << " not found in database\n";
    return -1;
  }


  std::cout << "Checking segments disabled state\n";
  auto rseg = session->get_segment();
  if (!rseg->disabled(*session)) {
    std::cout << "Root segment " << rseg->UID()
              << " is not disabled, looping over contained segments\n";
    for (auto seg : rseg->get_segments()) {
      std::cout << "Segment " << seg->UID()
                << std::string(seg->disabled(*session)? " is ":" is not ")
                << "disabled\n";
    }
  }

  auto disabled = session->get_disabled();
  std::cout << "======\nCurrently " << disabled.size() << " items disabled: ";
  for (auto item : disabled) {
    std::cout << " " << item->UID();
  }
  std::cout << std::endl;
  listApps(session);

  std::cout << "======\nNow trying to set enabled  \n";
  std::set<const confmodel::Component*> enable;
  for (auto item : disabled) {
    enable.insert(item);
  }
  session->set_enabled(enable);
  listApps(session);

  std::cout << "======\nNow trying to set enabled to an empty list\n";
  enable.clear();
  session->set_enabled(enable);
  listApps(session);

  std::cout << "======\nNow trying to set disabled to an empty list \n";
  session->set_disabled({});
  listApps(session);

}
