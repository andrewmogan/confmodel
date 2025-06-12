#include "confmodel/ResourceBase.hpp"
#include "confmodel/ResourceHolder.hpp"
#include "confmodel/ResourceSet.hpp"

#include "confmodel/confmodelIssues.hpp"

#include "logging/Logging.hpp"

#include "test_circular_dependency.hpp"

using namespace dunedaq::confmodel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace dunedaq::confmodel;

namespace {
// fill data from resource sets
void fill(const ResourceSet& rs,
          std::vector<const ResourceSet*>& all_resource_sets,
          TestCircularDependency& cd_fuse) {
  TLOG_DEBUG(6) << "rs.UID=" << rs.UID() << ", class=" << rs.class_name();
  all_resource_sets.push_back(&rs);
  auto rptr = &rs;
  if (rptr->cast<ResourceBase>() == nullptr) {
    throw (MissingConstructor(ERS_HERE, "ResourceBase", rs.full_name()));
  }
  for (auto & res : rs.get_resources()) {
    AddTestOnCircularDependency add_fuse_test(cd_fuse, res);
    if (const ResourceSet * rs2 = res->cast<ResourceSet>()) {
      fill(*rs2, all_resource_sets, cd_fuse);
    }
  }
}

void disable_children(const ResourceSet& rs) {
  TLOG_DEBUG(6) << "Disabling children of " << rs.UID();
  for (auto & res : rs.get_resources()) {
    TLOG_DEBUG(6) << "Disabling child " << res->UID();
    disable(*res);
    if (const auto * rs2 = res->cast<ResourceSet>()) {
      disable_children(*rs2);
    }
  }
}

} // namespace

ResourceHolder::ResourceHolder(const ResourceSet* root,
                               std::vector<const ResourceBase*> initial_list) {
  m_disabled_items = update(root, initial_list);
}

std::set<std::string> ResourceHolder::update(const ResourceSet* root,
                            std::vector<const ResourceBase*> initial_list) {

  std::set<std::string> disabled;

  if (initial_list.empty()) {
    TLOG_DEBUG( 6) << "We have no disabled components";
    return;
  }

  // get list of all root's resource-sets also test any
  // circular dependencies between segments and resource sets
  TestCircularDependency cd_fuse("component \'is-disabled\' status", root);
  std::vector<const ResourceSet*> resource_sets;
  fill(*root, resource_sets, cd_fuse);

  for (auto & comp : initial_list) {
    disable(*comp);
    TLOG_DEBUG(6) << comp->UID() << " is disabled in session";
    if (const ResourceSet * rs = comp->cast<ResourceSet>()) {
      disable_children(*rs);
    }
  }

  for (unsigned long count = 1; true; ++count) {
    const unsigned long num(disabled.size()); // Remember current size

    TLOG_DEBUG(6) <<  "before auto-disabling iteration " << count << " the number of disabled components is " << num ;
    for (const auto& res_set : resource_sets) {
      if (is_enabled(res_set)) {
        if (res_set->is_disabled(disabled)) {
          TLOG_DEBUG(6) <<  "disable custom resource-set- " << res_set->UID() << " because children are disabled" ;
          disable(*res_set);
          disable_children(*res_set);
        }
      }
    }

    if (disabled.size() == num) {
      TLOG_DEBUG(6) <<  "after " << count << " iteration(s) auto-disabling algorithm found no newly disabled sets, exiting loop ..." ;
      break;
    }

    unsigned int iLimit(1000);
    if (count > iLimit) {
      ers::error(ReadMaxAllowedIterations(ERS_HERE, iLimit));
      break;
    }
  }
  return disabled;
}

