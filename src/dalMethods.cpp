/**
 * @file dalMethods.cxx
 *
 * Implementations of Methods defined in confmodel schema classes
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "confmodel/Application.hpp"
#include "confmodel/confmodelIssues.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/DetDataSender.hpp"
#include "confmodel/DetDataReceiver.hpp"
#include "confmodel/DetSenderSet.hpp"
#include "confmodel/DetectorToDaqConnection.hpp"
#include "confmodel/DetectorStream.hpp"
#include "confmodel/Jsonable.hpp"
#include "confmodel/OpMonURI.hpp"
#include "confmodel/PhysicalHost.hpp"
#include "confmodel/RCApplication.hpp"
#include "confmodel/Resource.hpp"
#include "confmodel/ResourceBase.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/ResourceSetAND.hpp"
#include "confmodel/ResourceSetOR.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"
#include "confmodel/Service.hpp"
#include "confmodel/VirtualHost.hpp"

#include "test_circular_dependency.hpp"

#include "nlohmann/json.hpp"
#include "conffwk/ConfigObject.hpp"
#include "conffwk/Configuration.hpp"
#include "conffwk/Schema.hpp"

#include <list>
#include <set>
#include <iostream>

// Stolen from ATLAS dal package
using namespace dunedaq::conffwk;

namespace dunedaq::confmodel {
  /**
   *  Static function to calculate list of components
   *  from the root segment to the lowest component which
   *  the child object (a segment or a resource) belongs.
   */

static void
make_parents_list(
    const ConfigObjectImpl * child,
    const dunedaq::confmodel::ResourceSet * resource_set,
    std::vector<const dunedaq::confmodel::ResourceBase *> & p_list,
    std::list< std::vector<const dunedaq::confmodel::ResourceBase *> >& out,
    dunedaq::confmodel::TestCircularDependency& cd_fuse)
{
  dunedaq::confmodel::AddTestOnCircularDependency add_fuse_test(cd_fuse, resource_set);

  // add the resource set to the path
  p_list.push_back(resource_set);

  // check if the application is in the resource relationship, i.e. is a resource or belongs to resource set(s)
  for (const auto& i : resource_set->get_contains()) {
    if (i->config_object().implementation() == child) {
      out.push_back(p_list);
    }
    else if (const dunedaq::confmodel::ResourceSet * rs = i->cast<dunedaq::confmodel::ResourceSet>()) {
      make_parents_list(child, rs, p_list, out, cd_fuse);
    }
  }

  // remove the resource set from the path
  p_list.pop_back();
}

static void
make_parents_list(
    const ConfigObjectImpl * child,
    const dunedaq::confmodel::Segment * segment,
    std::vector<const dunedaq::confmodel::ResourceBase *> & p_list,
    std::list<std::vector<const dunedaq::confmodel::ResourceBase *> >& out,
    bool is_segment,
    dunedaq::confmodel::TestCircularDependency& cd_fuse)
{
  dunedaq::confmodel::AddTestOnCircularDependency add_fuse_test(cd_fuse, segment);

  // add the segment to the path
  p_list.push_back(segment);

  // check if the application is in the nested segment
  for (const auto& seg : segment->get_segments()) {
    if (seg->config_object().implementation() == child)
      out.push_back(p_list);
    else
      make_parents_list(child, seg, p_list, out, is_segment, cd_fuse);
  }
  if (!is_segment) {
    for (const auto& app : segment->get_applications()) {
      if (app->config_object().implementation() == child)
        out.push_back(p_list);
      else if (const auto resource_set = app->cast<dunedaq::confmodel::ResourceSet>())
        make_parents_list(child, resource_set, p_list, out, cd_fuse);
    }
  }

  // remove the segment from the path

  p_list.pop_back();
}


static void
check_segment(
    std::list< std::vector<const dunedaq::confmodel::ResourceBase *> >& out,
    const dunedaq::confmodel::Segment * segment,
    const ConfigObjectImpl * child,
    bool is_segment,
    dunedaq::confmodel::TestCircularDependency& cd_fuse)
{
  dunedaq::confmodel::AddTestOnCircularDependency add_fuse_test(cd_fuse, segment);

  std::vector<const dunedaq::confmodel::ResourceBase *> compList;

  if (segment->config_object().implementation() == child) {
    out.push_back(compList);
  }
  make_parents_list(child, segment, compList, out, is_segment, cd_fuse);
}

void
dunedaq::confmodel::ResourceBase::get_parents(
  const dunedaq::confmodel::Session& session,
  std::list<std::vector<const dunedaq::confmodel::ResourceBase *>>& parents) const
{
  const ConfigObjectImpl * obj_impl = config_object().implementation();

  const bool is_segment = castable(dunedaq::confmodel::Segment::s_class_name);

  try {
    dunedaq::confmodel::TestCircularDependency cd_fuse("component parents", &session);

    // check session's segment
    check_segment(parents, session.get_segment(), obj_impl, is_segment,
                  cd_fuse);


    if (parents.empty()) {
      TLOG_DEBUG(1) <<  "cannot find segment/resource path(s) between ResourceBase " << this << " and session " << &session << " objects (check this object is linked with the session as a segment or a resource)" ;
    }
  }
  catch (ers::Issue & ex) {
    ers::error(CannotGetParents(ERS_HERE, full_name(), ex));
  }
}

// ========================================================================

  static std::vector<const Application*> getSegmentApps(const Segment* segment,
                                                        const Session* session,
                                                        bool enabled_only) {
  std::vector<const Application*> apps;
  auto segapps = segment->get_applications();
  if (enabled_only) {
    for (auto app : segapps) {
      auto comp = app->cast<ResourceBase>();
      if (comp == nullptr || !comp->disabled(*session)) {
        apps.insert(apps.end(), app);
      }
    }
  }
  else {
    apps.swap(segapps);
  }
  for (auto seg : segment->get_segments()) {
    if (!enabled_only || !seg->disabled(*session)) {
      auto segapps = getSegmentApps(seg, session, enabled_only);
      apps.insert(apps.end(), segapps.begin(),segapps.end());
    }
  }
  return apps;
}

std::vector<const Application*>
Session::get_all_applications() const {
  std::vector<const Application*> apps;
  auto segapps = getSegmentApps(m_segment, this, false);
  apps.insert(apps.end(), segapps.begin(),segapps.end());
  return apps;
}

std::vector<const Application*>
Session::get_enabled_applications() const {
  std::vector<const Application*> apps;
  auto segapps = getSegmentApps(m_segment, this, true);
  apps.insert(apps.end(), segapps.begin(),segapps.end());
  return apps;
}

// ========================================================================

std::set<const HostComponent*>
DaqApplication::get_used_hostresources() const {
  std::set<const HostComponent*> res;
  for (auto module :  get_modules()) {
    for (auto hostresource : module->get_used_resources()) {
      res.insert(hostresource);
    }
  }
  return res;
}

nlohmann::json get_json_config(conffwk::Configuration& confdb,
                               const std::string& class_name,
                               const std::string& uid,
                               bool direct_only) {
  using nlohmann::json;
  using namespace conffwk;
  TLOG_DBG(9) << "Getting attributes for " << uid << " of class " << class_name;
  json attributes;
  auto class_info = confdb.get_class_info(class_name);
  ConfigObject obj;
  confdb.get(class_name, uid, obj);
  for (auto attr : class_info.p_attributes) {
    if (attr.p_type == type_t::u8_type) {
      add_json_value<uint8_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::u16_type) {
      add_json_value<uint16_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::u32_type) {
      add_json_value<uint32_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::u64_type) {
      add_json_value<uint64_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s8_type) {
      add_json_value<int8_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s16_type) {
      add_json_value<int16_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s32_type ||
             attr.p_type == type_t::s16_type) {
      add_json_value<int32_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::s64_type) {
      add_json_value<int64_t>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::float_type) {
      add_json_value<float>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::double_type) {
      add_json_value<double>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if (attr.p_type == type_t::bool_type) {
      add_json_value<bool>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
    else if ((attr.p_type == type_t::string_type) ||
             (attr.p_type == type_t::enum_type) ||
             (attr.p_type == type_t::date_type) ||
             (attr.p_type == type_t::time_type)) {
      add_json_value<std::string>(obj, attr.p_name, attr.p_is_multi_value, attributes);
    }
  }
  if (!direct_only) {
    TLOG_DBG(9) << "Processing  relationships";
    for (auto iter: class_info.p_relationships) {
      std::string rel_name = iter.p_name;
      if (iter.p_cardinality == cardinality_t::zero_or_one ||
          iter.p_cardinality == cardinality_t::only_one) {
        ConfigObject rel_obj;
        obj.get(rel_name, rel_obj);
        if (!rel_obj.is_null()) {
          TLOG_DBG(9) << "Getting attibute of relationship " << rel_name;
          attributes[rel_name] = get_json_config(confdb, rel_obj.class_name(),
                                                 rel_obj.UID(),
                                                 direct_only);
        }
        else {
          TLOG_DBG(9) << "Relationship " << rel_name << " not set";
        }
      }
      else {
        TLOG_DBG(9) << "Relationship " << rel_name << " is multi value. "
                    << "Getting attibutes for relationship.";
        std::vector<ConfigObject> rel_vec;
        obj.get(rel_name, rel_vec);
        std::vector<json> configs;
        for (auto rel_obj : rel_vec) {
          TLOG_DBG(9) << "Getting attibute of relationship " << rel_obj.UID();
          auto rel_conf = get_json_config(confdb, rel_obj.class_name(), rel_obj.UID(),
                                          direct_only);
          configs.push_back(rel_conf);
        }
        attributes[rel_name] = configs;
      }
    }
  }
  json json_config;
  json_config[uid] = attributes;
  return json_config;
}

nlohmann::json Jsonable::to_json(bool direct_only) const {

  return get_json_config(p_registry.configuration(), class_name(), UID(), direct_only);
}

const std::vector<std::string> DaqApplication::construct_commandline_parameters(
  const conffwk::Configuration& confdb,
  const dunedaq::confmodel::Session* session) const {

    return construct_commandline_parameters_appfwk<dunedaq::confmodel::DaqApplication>(this, confdb, session);
}

const std::vector<std::string> RCApplication::construct_commandline_parameters(
  const conffwk::Configuration& confdb,
  const dunedaq::confmodel::Session* session) const {

    const std::string configuration_uri = confdb.get_impl_spec();
    const dunedaq::confmodel::Service* control_service = nullptr;

    const std::string controller_log_level = session->get_controller_log_level();

    for (auto const* as: get_exposes_service())
      if (as->UID() == UID()+"_control") // unclear this is the best way to do this.
        control_service = as;

    if (control_service == nullptr)
      throw NoControlServiceDefined(ERS_HERE, UID());

    const std::string control_uri =
      control_service->get_protocol()
      + "://"
      + get_runs_on()->get_runs_on()->UID()
      + ":"
      + std::to_string(control_service->get_port());

    std::vector<std::string> ret = { "-l", controller_log_level };
    ret.push_back(configuration_uri);
    ret.push_back(control_uri);
    ret.push_back(UID());
    ret.push_back(session->UID());
    return ret;
}



std::vector<const confmodel::DetectorStream*> DetectorToDaqConnection::get_streams() const {

  std::vector<const confmodel::DetectorStream*> streams;
    // Loop over senders
  for (auto sender : this->get_senders()->get_senders()) {
      // loop over streams
      for (auto stream_res : sender->get_streams()) {
        streams.push_back(stream_res);
      }
  }
  return streams;
}

std::string OpMonURI::get_URI( const std::string & /* app */) const {

  auto type = get_type();
  if ( type == "file" ) { 
    return type + "://" + get_path();
  }
  
  if ( type == "stream" ) {
    return type + "://" + get_path();
  }
  
  return "stdout://";  
}


const std::vector<const ResourceBase*>& ResourceSet::get_contains() const {
  throw (BadConf(ERS_HERE,
                 "No get_contains method defined for ResourceSet"));
}

const std::vector<const ResourceBase*>& DetSenderSet::get_contains() const {
  if (m_contents.empty()) {
    std::lock_guard scoped_lock(m_mutex);
    check_init();
    for (auto sender: m_senders) {
      m_contents.push_back(sender);
    }
  }
  return m_contents;
}
bool DetSenderSet::disabled(const Session& session) const {
  for (auto sender: m_senders) {
    if (!sender->disabled(session)) {
      return false;
    }
  }
  return true;
}


const std::vector<const ResourceBase*>& DetDataSender::get_contains() const {
  if (m_contents.empty()) {
    std::lock_guard scoped_lock(m_mutex);
    check_init();
    for (auto stream: m_streams) {
      m_contents.push_back(dynamic_cast<const ResourceBase*>(stream));
    }
  }
  return m_contents;
}
bool DetDataSender::disabled(const Session& session) const {
  for (auto stream: m_streams) {
    if (!stream->disabled(session)) {
      return false;
    }
  }
  return true;
}


const std::vector<const ResourceBase*>& DetectorToDaqConnection::get_contains() const {
  TLOG_DBG(6) << "m_contents.size=" << m_contents.size()
              << " m_senders=" << m_senders
              << " m_receiver=" << m_receiver;
  if (m_contents.empty()) {
    std::lock_guard scoped_lock(m_mutex);
    check_init();
    m_contents.push_back(m_senders);
    m_contents.push_back(m_receiver);
  }
  return m_contents;
}

bool DetectorToDaqConnection::disabled(const Session& session) const {
  TLOG_DBG(6) << "receiver disabled=" << get_receiver()->disabled(session)
         << " senders disabled=" << get_senders()->disabled(session);
  if (get_receiver()->disabled(session) || get_senders()->disabled(session)) {
    return true;
  }
  return false;
}

} // namespace dunedaq::confmodel
