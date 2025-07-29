/**
 * @file DisabledResource_test.cxx  Unit Tests for Resource disabling logic
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2025.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "conffwk/Configuration.hpp"
#include "confmodel/DisabledResources.hpp"
#include "confmodel/DummyApplication.hpp"
#include "confmodel/DummyD2D.hpp"
#include "confmodel/DummyReceiver.hpp"
#include "confmodel/DummyResource.hpp"
#include "confmodel/DummyResourceSetAND.hpp"
#include "confmodel/DummyResourceSet.hpp"
#include "confmodel/DummySender.hpp"
#include "confmodel/DummyStream.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include "logging/Logging.hpp"

#define BOOST_TEST_MODULE DisabledResource_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <list>
#include <string>


BOOST_AUTO_TEST_SUITE(DisabledResource_test)

using namespace dunedaq;
using namespace dunedaq::confmodel;


BOOST_AUTO_TEST_CASE(simple_resource_set){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_resource.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummyResource*> dummy_resources;
  std::vector<const conffwk::ConfigObject*> resource_config_objects;
  for (std::string id : {"dummyRes-0", "dummyRes-1", "dummyRes-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummyResource", id, conf_obj);
    auto res_dal = confdb.get<DummyResource>(conf_obj);
    resource_config_objects.push_back(&res_dal->config_object());
    dummy_resources.push_back(res_dal);
  }

  conffwk::ConfigObject conf_obj;
  confdb.create(oksfile, "DummyResourceSet", "root", conf_obj);
  conf_obj.set_objs("items", resource_config_objects);

  auto root = confdb.get<DummyResourceSet>(conf_obj);

  // Nothing disabled
  DisabledResources dr(root,{});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( dr.is_enabled(dummy_resources[1]) );

  // Single resource disabled
  dr.update(root,{dummy_resources[1]});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( dr.is_enabled(dummy_resources[0]) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[1]) );

  // All simple resources disabled - no affect on ResourceSet
  dr.update(root, {dummy_resources[0], dummy_resources[1], dummy_resources[2]});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[1]) );

  // ResourceSet disabled -- should affect contained simple Resources
  dr.update(root,{root});
  BOOST_CHECK( !dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[0]) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[1]) );

}

BOOST_AUTO_TEST_CASE(resource_set_and){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_resource.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummyResource*> dummy_resources;
  std::vector<const conffwk::ConfigObject*> resource_config_objects;
  for (std::string id : {"dummyRes-0", "dummyRes-1", "dummyRes-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummyResource", id, conf_obj);
    auto res_dal = confdb.get<DummyResource>(conf_obj);
    resource_config_objects.push_back(&res_dal->config_object());
    dummy_resources.push_back(res_dal);
  }

  conffwk::ConfigObject conf_obj;
  confdb.create(oksfile, "DummyResourceSetAND", "root", conf_obj);
  conf_obj.set_objs("items", resource_config_objects);

  auto root = confdb.get<DummyResourceSetAND>(conf_obj);

  // Nothing disabled
  DisabledResources dr(root,{});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( dr.is_enabled(dummy_resources[1]) );

  // Single resource disabled
  dr.update(root,{dummy_resources[1]});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( dr.is_enabled(dummy_resources[0]) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[1]) );

  // All simple resources disabled - also disables ResourceSetDisableAND
  dr.update(root, {dummy_resources[0], dummy_resources[1], dummy_resources[2]});
  BOOST_CHECK( !dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[1]) );

  // ResourceSet disabled -- should affect contained simple Resources
  dr.update(root,{root});
  BOOST_CHECK( !dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[0]) );
  BOOST_CHECK( !dr.is_enabled(dummy_resources[1]) );

}


BOOST_AUTO_TEST_CASE(segment){
  dunedaq::logging::Logging().setup("a","a");
  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_resource.schema.xml"};
  confdb.create(oksfile, includes);

  std::vector<const DummyApplication*> dummy_apps;
  std::vector<const conffwk::ConfigObject*> app_config_objects;
  for (std::string id : {"dummyApp-0", "dummyApp-1", "dummyApp-2"}) {
    conffwk::ConfigObject conf_obj;
    confdb.create(oksfile, "DummyApplication", id, conf_obj);
    auto res_dal = confdb.get<DummyApplication>(conf_obj);
    app_config_objects.push_back(&res_dal->config_object());
    dummy_apps.push_back(res_dal);
  }

  conffwk::ConfigObject segment_conf_obj;
  confdb.create(oksfile, "Segment", "root", segment_conf_obj);
  segment_conf_obj.set_objs("applications", app_config_objects);

  auto root = confdb.get<Segment>(segment_conf_obj);

  // Nothing disabled
  DisabledResources dr(root,{});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( dr.is_enabled(dummy_apps[0]) );

  // Single resource disabled
  dr.update(root,{dummy_apps[0]});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(dummy_apps[0]) );
  BOOST_CHECK( dr.is_enabled(dummy_apps[1]) );

  // All simple resources disabled - also disables Segment (ResourceSetDisableAND)
  dr.update(root, {dummy_apps[0], dummy_apps[1], dummy_apps[2]});
  BOOST_CHECK( !dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(dummy_apps[1]) );
}


BOOST_AUTO_TEST_CASE(detector_to_daq){

  conffwk::Configuration confdb("oksconflibs");
  const std::string oksfile{"/tmp/drtest.data.xml"};
  const std::list<std::string> includes{
    "schema/confmodel/dunedaq.schema.xml",
    "schema/confmodel/dummy_resource.schema.xml"};
  confdb.create(oksfile, includes);

  conffwk::ConfigObject conf_obj;

  std::vector<const Resource*> sender0_streams;
  std::vector<const conffwk::ConfigObject*> stream_config_objects;
  for (std::string id : {"dummyStream-0", "dummyStream-1", "dummyStream-2"}) {
    confdb.create(oksfile, "DummyStream", id, conf_obj);
    auto res_dal = confdb.get<DummyStream>(conf_obj);
    stream_config_objects.push_back(&res_dal->config_object());
    sender0_streams.push_back(res_dal);
  }


  std::vector<const conffwk::ConfigObject*> sender_conf_objs;
  confdb.create(oksfile, "DummySender", "sender-0", conf_obj);
  conf_obj.set_objs("streams", stream_config_objects);
  auto sender0_dal = confdb.get<DummySender>(conf_obj);
  sender_conf_objs.push_back(&sender0_dal->config_object());


  std::vector<const Resource*> sender1_streams;
  stream_config_objects.clear();
  for (std::string id : {"dummyStream-3", "dummyStream-4", "dummyStream-5"}) {
    confdb.create(oksfile, "DummyStream", id, conf_obj);
    auto res_dal = confdb.get<DummyStream>(conf_obj);
    stream_config_objects.push_back(&res_dal->config_object());
    sender1_streams.push_back(res_dal);
  }

  confdb.create(oksfile, "DummySender", "sender-1", conf_obj);
  conf_obj.set_objs("streams", stream_config_objects);
  auto sender1_dal = confdb.get<DummySender>(conf_obj);
  sender_conf_objs.push_back(&sender1_dal->config_object());

  confdb.create(oksfile, "DummyReceiver", "receiver-0", conf_obj);
  auto receiver = confdb.get<DummyReceiver>(conf_obj);
  auto receiver_dal = confdb.get<DummyReceiver>(conf_obj);
  auto receiver_conf_obj = receiver_dal->config_object();

  confdb.create(oksfile, "DummyD2D", "d2d-0", conf_obj);
  conf_obj.set_objs("dummy_senders", sender_conf_objs);
  conf_obj.set_obj("dummy_receiver", &receiver_conf_obj);
  auto d2d_dal = confdb.get<DummyD2D>(conf_obj);
  auto d2d_conf_obj = d2d_dal->config_object();
  confdb.create(oksfile, "DummyResourceSet", "root", conf_obj);
  conf_obj.set_objs("items", {&d2d_conf_obj});

  auto root = confdb.get<DummyResourceSet>(conf_obj);

  // Nothing disabled
  DisabledResources dr(root,{});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( dr.is_enabled(receiver_dal) );
  BOOST_CHECK( dr.is_enabled(d2d_dal) );

  // receiver disabled - disables d2d
  dr.update(root,{receiver});
  BOOST_CHECK( dr.is_enabled(root) );
  BOOST_CHECK( !dr.is_enabled(receiver_dal) );
  BOOST_CHECK( !dr.is_enabled(d2d_dal) );

  
  std::vector<const Resource*> disable = sender0_streams;
  // All streams of sender0 disabled - disables sender0
  dr.update(root, disable);
  BOOST_CHECK( dr.is_enabled(d2d_dal) );
  BOOST_CHECK( dr.is_enabled(sender1_dal) );
  BOOST_CHECK( !dr.is_enabled(sender0_dal) );
  BOOST_CHECK( dr.is_enabled(receiver_dal) );

  // All streams of both senders disabled - disables d2d and in turn receiver
  disable.insert(disable.end(), sender1_streams.begin(), sender1_streams.end());
  dr.update(root, disable);
  BOOST_CHECK( !dr.is_enabled(d2d_dal) );
  BOOST_CHECK( !dr.is_enabled(receiver_dal) );

  // Sender0 and all streams of sender1 disabled - also disables d2d
  disable.clear();
  disable.push_back(sender0_dal);
  disable.insert(disable.end(), sender1_streams.begin(), sender1_streams.end());
  dr.update(root, disable);
  BOOST_CHECK( !dr.is_enabled(d2d_dal) );
  BOOST_CHECK( !dr.is_enabled(receiver_dal) );

  // Both senders disabled - same as above
  disable.clear();
  disable.push_back(sender0_dal);
  disable.push_back(sender1_dal);
  dr.update(root, disable);
  BOOST_CHECK( !dr.is_enabled(d2d_dal) );
  BOOST_CHECK( !dr.is_enabled(receiver_dal) );

}



BOOST_AUTO_TEST_SUITE_END()
