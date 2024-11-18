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

namespace dunedaq {

ERS_DECLARE_ISSUE(confmodel, InvalidOpMonFile,
                  file_name << " is an invalid name for the opmon output",
                  ((std::string)file_name))

ERS_DECLARE_ISSUE(confmodel, ConfigurationError, , )

ERS_DECLARE_ISSUE_BASE(
    confmodel, NoOpmonInfrastructure, ConfigurationError,
    "The opmon infrastructure has not been set up in the configuration", , )

ERS_DECLARE_ISSUE_BASE(
    confmodel, NoControlServiceDefined, ConfigurationError,
    "The control service has not been set up for the application " + app_name +
        " you need to define a service called " + app_name + "_control",
    , ((std::string)app_name)

)

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

ERS_DECLARE_ISSUE(confmodel, AlgorithmError, , )

ERS_DECLARE_ISSUE_BASE(confmodel, BadVariableUsage, AlgorithmError, message, ,
                       ((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, BadApplicationInfo, AlgorithmError,
                       "Failed to retrieve information for Application \'"
                           << app_id << "\' from the database: " << message,
                       , ((std::string)app_id)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, BadSessionID, AlgorithmError,
                       "There is no session object with UID = \"" << name
                                                                  << '\"',
                       , ((std::string)name))

ERS_DECLARE_ISSUE_BASE(
    confmodel, SegmentDisabled, AlgorithmError,
    "Cannot get information about applications because the segment is disabled",
    , )

ERS_DECLARE_ISSUE_BASE(confmodel, BadProgramInfo, AlgorithmError,
                       "Failed to retrieve information for Program \'"
                           << prog_id << "\' from the database: " << message,
                       , ((std::string)prog_id)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, BadHost, AlgorithmError,
                       "Failed to retrieve application \'"
                           << app_id << "\' from the database: " << message,
                       , ((std::string)app_id)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, NoDefaultHost, AlgorithmError,
                       "Failed to find default host for segment \'"
                           << seg_id << "\' " << message,
                       , ((std::string)seg_id)((std::string)message))

ERS_DECLARE_ISSUE_BASE(
    confmodel, NoTemplateAppHost, AlgorithmError,
    "Both session default and segment default hosts are not defined for "
    "template application \'"
        << app_id << "\' from segment \'" << seg_id
        << "\' (will use localhost, that may cause problems presenting info in "
           "IGUI for distributed session).",
    , ((std::string)app_id)((std::string)seg_id))

ERS_DECLARE_ISSUE_BASE(confmodel, BadTag, AlgorithmError,
                       "Failed to use tag \'" << tag_id
                                              << "\' because: " << message,
                       , ((std::string)tag_id)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, BadSegment, AlgorithmError,
                       "Invalid Segment \'" << seg_id
                                            << "\' because: " << message,
                       , ((std::string)seg_id)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, GetTemplateApplicationsOfSegmentError,
                       AlgorithmError,
                       "Failed to get template applications of \'"
                           << name << "\' segment" << message,
                       , ((std::string)name)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, BadTemplateSegmentDescription, AlgorithmError,
                       "Bad configuration description of template segment \'"
                           << name << "\': " << message,
                       , ((std::string)name)((std::string)message))

ERS_DECLARE_ISSUE_BASE(confmodel, CannotGetApplicationObject, AlgorithmError,
                       "Failed to get application object from name: " << reason,
                       , ((std::string)reason))

ERS_DECLARE_ISSUE_BASE(confmodel, CannotFindSegmentByName, AlgorithmError,
                       "Failed to find segment object \'" << name
                                                          << "\': " << reason,
                       , ((std::string)name)((std::string)reason))

ERS_DECLARE_ISSUE_BASE(confmodel, NotInitedObject, AlgorithmError,
                       "The " << item << " object " << obj
                              << " was not initialized",
                       , ((const char *)item)((void *)obj))

ERS_DECLARE_ISSUE_BASE(confmodel, NotInitedByDalAlgorithm, AlgorithmError,
                       "The " << obj_id << '@' << obj_class << " object "
                              << address
                              << " was not initialized by DAL algorithm "
                              << algo,
                       ,
                       ((std::string)obj_id)((std::string)obj_class)(
                           (void *)address)((const char *)algo))

ERS_DECLARE_ISSUE_BASE(confmodel, CannotCreateSegConfig, AlgorithmError,
                       "Failed to create config for segment \'"
                           << name << "\': " << reason,
                       , ((std::string)name)((std::string)reason))

ERS_DECLARE_ISSUE_BASE(confmodel, CannotGetParents, AlgorithmError,
                       "Failed to get parents of \'" << object << '\'', ,
                       ((std::string)object))

ERS_DECLARE_ISSUE_BASE(
    confmodel, FoundCircularDependency, AlgorithmError,
    "Reach maximum allowed recursion ("
        << limit << ") during calculation of " << goal
        << "; possibly there is circular dependency between these objects: "
        << objects,
    , ((unsigned int)limit)((const char *)goal)((std::string)objects))

ERS_DECLARE_ISSUE_BASE(
    confmodel, NoJarFile, AlgorithmError,
    "Cannot find jar file \'" << file << "\' described by \'" << obj_id << '@'
                              << obj_class << "\' that is part of \'" << rep_id
                              << '@' << rep_class << '\'',
    ,
    ((std::string)file)((std::string)obj_id)((std::string)obj_class)(
        (std::string)rep_id)((std::string)rep_class))

ERS_DECLARE_ISSUE_BASE(confmodel, DuplicatedApplicationID, AlgorithmError,
                       "Two applications have equal IDs:\n  1) "
                           << first << "\n  2) " << second,
                       , ((std::string)first)((std::string)second))

ERS_DECLARE_ISSUE_BASE(
    confmodel, SegmentIncludedMultipleTimes, AlgorithmError,
    "The segment \"" << segment << "\" is included by:\n  1) " << first
                     << "\n  2) " << second,
    , ((std::string)segment)((std::string)first)((std::string)second))

} // namespace dunedaq

#endif
