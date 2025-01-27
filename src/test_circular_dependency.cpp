#include "conffwk/DalObject.hpp"
#include "confmodel/confmodelIssues.hpp"
#include "test_circular_dependency.hpp"

void
dunedaq::confmodel::TestCircularDependency::push(const dunedaq::conffwk::DalObject * object)
{
  if(p_index < p_limit) {
    p_objects[p_index++] = object;
  }
  else {
    std::ostringstream s;
    for(unsigned int i = 0; i < p_index; ++i) {
      if(i != 0) s << ", ";
      s << p_objects[i];
    }

    throw dunedaq::confmodel::FoundCircularDependency(ERS_HERE, p_limit, p_goal, s.str());
  }
}
