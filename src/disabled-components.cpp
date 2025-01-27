#include "confmodel/Application.hpp"
#include "confmodel/ResourceBase.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include "confmodel/confmodelIssues.hpp"
#include "confmodel/disabled-components.hpp"

#include "logging/Logging.hpp"

#include "test_circular_dependency.hpp"

#include <dlfcn.h>

using namespace dunedaq::confmodel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DisabledResources::DisabledResources(Session& session)
{
  TLOG_DEBUG(2) <<  "construct the object " << (void *)this ;

  // Check that the dal is available for all schema used by the Session

  // Extract unique list of package names from schema file names
  std::set<std::string> packages;
  for (auto file: session.configuration().get_schema()) {
    std::string search {"schema/"};
    auto start = file.rfind(search) + search.size();
    auto end = file.find("/", start);
    packages.insert(file.substr(start,end-start));
  }
  // Try loading the package's shared library
  for (auto package: packages) {
    std::string library = "lib"+package+".so";
    auto handle = dlopen(library.c_str(), RTLD_LAZY|RTLD_GLOBAL);
    if (handle == nullptr) {
      throw (LoadDalFailed(ERS_HERE, library));
    }
  }

  if (session.get_disabled().empty()) {
    TLOG_DEBUG( 6) << "Session has no disabled components";
    return;
  }

  // get list of all session's resource-sets also test any
  // circular dependencies between segments and resource sets
  TestCircularDependency cd_fuse("component \'is-disabled\' status", &session);
  std::vector<const ResourceSet*> resource_sets;
  auto seg = dynamic_cast<const ResourceSet*>(session.get_segment());
  fill(*seg, resource_sets, cd_fuse);


  for (auto & comp : session.get_disabled()) {
    disable(*comp);
    TLOG_DEBUG(6) << comp->UID() << " is disabled in session";
    if (const ResourceSet * rs = comp->cast<ResourceSet>()) {
      disable_children(*rs);
    }
  }

  for (unsigned long count = 1; true; ++count) {
    const unsigned long num(size()); // Remember current size

    TLOG_DEBUG(6) <<  "before auto-disabling iteration " << count << " the number of disabled components is " << num ;
    for (const auto& res_set : resource_sets) {
      if (is_enabled(res_set)) {
        if (res_set->is_disabled(m_disabled)) {
          TLOG_DEBUG(6) <<  "disable custom resource-set- " << res_set->UID() << " because children are disabled" ;
          disable(*res_set);
          disable_children(*res_set);
        }
      }
    }

    if (size() == num) {
      TLOG_DEBUG(6) <<  "after " << count << " iteration(s) auto-disabling algorithm found no newly disabled sets, exiting loop ..." ;
      break;
    }

    unsigned int iLimit(1000);
    if (count > iLimit) {
      ers::error(ReadMaxAllowedIterations(ERS_HERE, iLimit));
      break;
    }
  }
}

// fill data from resource sets
void DisabledResources::fill(const ResourceSet& rs,
                             std::vector<const ResourceSet*>& all_resource_sets,
                             TestCircularDependency& cd_fuse)
{
  TLOG_DEBUG(6) << "rs.UID=" << rs.UID() << ", class=" << rs.class_name();
  all_resource_sets.push_back(&rs);
  auto rptr = &rs;
  if (rptr->cast<ResourceBase>() == nullptr) {
    throw (MissingConstructor(ERS_HERE, "ResourceBase", rs.full_name()));
  }
  for (auto & res : rs.get_contains()) {
    AddTestOnCircularDependency add_fuse_test(cd_fuse, res);
    if (const ResourceSet * rs2 = res->cast<ResourceSet>()) {
      fill(*rs2, all_resource_sets, cd_fuse);
    }
  }
}



void
DisabledResources::disable_children(const ResourceSet& rs)
{
  TLOG_DEBUG(6) << "Disabling children of " << rs.UID();
  for (auto & res : rs.get_contains()) {
    TLOG_DEBUG(6) << "Disabling child " << res->UID();
    disable(*res);
    if (const auto * rs2 = res->cast<ResourceSet>()) {
      disable_children(*rs2);
    }
  }
}

