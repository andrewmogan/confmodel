#ifndef DUNEDAQDAL_DISABLED_RESOURCES_H
#define DUNEDAQDAL_DISABLED_RESOURCES_H

#include "confmodel/Resource.hpp"

#include <set>
#include <string>


namespace dunedaq::confmodel {

    class Session;
    class ResourceSet;
    class TestCircularDependency;

    class DisabledResources 
    {

      friend class Session;
      friend class Resource;

    private:

      std::set<std::string> m_disabled;
      bool m_initialised{false};
      void fill(const ResourceSet& rs,
                std::vector<const ResourceSet*>& all_resource_sets,
                TestCircularDependency& cd_fuse);

      void
      disable(const Resource& component)
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

      DisabledResources() = default;
      DisabledResources(const ResourceSet* root,
                        std::vector<const Resource*> initial_list);

      ~DisabledResources() = default;

      void update(const ResourceSet* root,
                  std::vector<const Resource*> initial_list);

      bool
      is_enabled(const Resource* component) const {
        return !m_disabled.contains(component->UID());
      }

      [[nodiscard]] bool initialised() const {return m_initialised;}
    };
} // namespace dunedaq::confmodel

#endif // DUNEDAQDAL_DISABLED_RESOURCES_H
