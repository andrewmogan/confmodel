#ifndef DUNEDAQDAL_DISABLED_COMPONENTS_H
#define DUNEDAQDAL_DISABLED_COMPONENTS_H

#include "confmodel/ResourceBase.hpp"

#include <set>
#include <string>


namespace dunedaq::confmodel {

    class Session;
    class ResourceSet;
    class TestCircularDependency;

    class DisabledResources 
    {

      friend class Session;
      friend class ResourceBase;

    private:

      std::set<std::string> m_disabled;

      void fill(const ResourceSet& rs,
                std::vector<const ResourceSet*>& all_resource_sets,
                TestCircularDependency& cd_fuse);

      void
      disable(const ResourceBase& component)
      {
        m_disabled.insert(component.UID());
      }

      void
      disable_children(const ResourceSet&);

      size_t
      size() noexcept
      {
        return m_disabled.size();
      }

    public:

      explicit DisabledResources(Session& session);
      DisabledResources(const ResourceSet* root,
                        std::vector<const ResourceBase*> initial_list);

      ~DisabledResources() = default;

      void update(const ResourceSet* root,
                  std::vector<const ResourceBase*> initial_list);

      bool
      is_enabled(const ResourceBase* component) {
        return !m_disabled.contains(component->UID());
      }

    };
} // namespace dunedaq::confmodel

#endif // DUNEDAQDAL_DISABLED_COMPONENTS_H
