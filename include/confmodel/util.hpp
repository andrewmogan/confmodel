#ifndef _dal_util_H_
#define _dal_util_H_

#include <exception>

#include "conffwk/Configuration.hpp"
// #include "conffwk/DalObject.hpp"
#include "nlohmann/json.hpp"

#include "confmodel/Application.hpp"
#include "confmodel/PhysicalHost.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Service.hpp"
#include "confmodel/Session.hpp"
#include "confmodel/VirtualHost.hpp"
#include "confmodel/confmodelIssues.hpp"

namespace dunedaq {

namespace confmodel {

/**
 *  \brief Get session object.
 *
 *  The algorithm %is searching the session object by given name.
 *  If the name %is empty, then the algorithm takes the name from
 *  the TDAQ_SESSION environment variable.<BR>
 *
 *  The last parameter of the algorithm can be used to optimise performance
 *  of the DAL in case if a database server config implementation %is used.
 *  The parameter defines how many layers of objects referenced by given
 *  session object should be read into client's config cache together with
 *  session object during single network operation. For example:
 *  - if the parameter %is 0, then only session object %is read;
 *  - if the parameter %is 1, then session and first layer segment objects are
 * read;
 *  - if the parameter %is 2, then session, segments of first and second layers,
 * and application/resources of first layer segments objects are read;
 *  - if the parameter %is 10, then mostly probable all objects referenced by
 * given session object are read.<BR>
 *
 *  The parameters of the algorithm are:
 *  \param conf      the configuration object with loaded database
 *  \param name      the name of the session to be loaded (if empty,
 * TDAQ_SESSION variable %is used) \param rlevel    optional parameter to
 * optimise performance ("the references level") \param rclasses  optional
 * parameter to optimise performance ("names of classes which objects are
 * cached")
 *
 *  \return Returns the pointer to the session object if found, or 0.
 */

const dunedaq::confmodel::Session *
get_session(dunedaq::conffwk::Configuration &conf, const std::string &name,
            unsigned long rlevel = 10,
            const std::vector<std::string> *rclasses = nullptr);

template <typename T>
void add_json_value(conffwk::ConfigObject &obj, std::string &name,
                    bool multi_value, nlohmann::json &attributes) {
  if (!multi_value) {
    T value;
    obj.get(name, value);
    attributes[name] = value;
  } else {
    std::vector<T> value_vector;
    obj.get(name, value_vector);
    attributes[name] = nlohmann::json(value_vector);
  }
}

template <typename T>
const std::vector<std::string> construct_commandline_parameters_appfwk(
    const T *app, const conffwk::Configuration &confdb,
    const dunedaq::confmodel::Session *session) {

  const dunedaq::confmodel::Service *control_service = nullptr;

  for (auto const *as : app->get_exposes_service())
    if (as->UID() ==
        app->UID() + "_control") // unclear this is the best way to do this.
      control_service = as;

  if (control_service == nullptr)
    throw NoControlServiceDefined(ERS_HERE, app->UID());

  const std::string control_uri = control_service->get_protocol() + "://" +
                                  app->get_runs_on()->get_runs_on()->UID() +
                                  ":" +
                                  std::to_string(control_service->get_port());

  const std::string configuration_uri = confdb.get_impl_spec();

  return {
      "-s",
      session->UID(),
      "--name",
      app->UID(),
      "-c",
      control_uri,
      "--configurationService",
      configuration_uri,
  };
}

} // namespace confmodel

} // namespace dunedaq

#endif
