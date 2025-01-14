#ifndef CONFMODELISSUES_HPP
#define CONFMODELISSUES_HPP

#include "ers/Issue.hpp"

namespace dunedaq {
  ERS_DECLARE_ISSUE(confmodel, BadConf, what, ((std::string)what))
  ERS_DECLARE_ISSUE(confmodel, BadStreamConf,
                    "Failed to cast stream parameters " << id << " to " << stype,
                    ((std::string)id) ((std::string)stype))
}


#endif // CONFMODELISSUES_HPP
