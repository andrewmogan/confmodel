#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Jsonable.hpp"
#include "confmodel/JsonableTest.hpp"

#include "nlohmann/json.hpp"

#include <iostream>
#include <string>
#include <list>

using namespace dunedaq;


int main(int argc, char* argv[]) {
  std::list<std::string> inc_files{"schema/confmodel/dunedaq.schema.xml"};
  conffwk::Configuration confdb ("oksconflibs");
  std::string dbfile{"/tmp/js-test.data.xml"};
  confdb.create(dbfile, inc_files);

  conffwk::ConfigObject config_obj1;
  confdb.create(dbfile, "JsonableTest", "test1", config_obj1);

  conffwk::ConfigObject config_obj2;
  confdb.create(dbfile, "JsonableTest", "test2", config_obj2);

  config_obj1.set_by_val<unsigned int>("this", 1);
  config_obj1.set_by_val<unsigned int>("that", 1);
  config_obj1.set_obj("other", &config_obj2);

  config_obj2.set_by_val<unsigned int>("this", 2);
  config_obj2.set_by_val<unsigned int>("that", 2);

  auto dal = confdb.get<confmodel::JsonableTest>("test1");
  nlohmann::json json1 = dal->to_json();
  std::cout << "json1:\n=====\n" << json1.dump(4) << "\n";

  nlohmann::json json2 = dal->to_json(false, true);
  std::cout << "json2:\n=====\n" << json2.dump(4) << "\n";

}
