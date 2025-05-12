#ifndef CONFMODELISSUES_HPP
#define CONFMODELISSUES_HPP

#include "ers/Issue.hpp"

namespace dunedaq {
ERS_DECLARE_ISSUE(confmodel, InvalidOpMonFile,
                  file_name << " is an invalid name for the opmon output",
                  ((std::string)file_name))

ERS_DECLARE_ISSUE(confmodel, LoadDalFailed,
                  "Could not load dal shared library " << library,
                  ((std::string)library))

ERS_DECLARE_ISSUE(confmodel, ConfigurationError, , )

ERS_DECLARE_ISSUE_BASE(
    confmodel, NoOpmonInfrastructure, ConfigurationError,
    "The opmon infrastructure has not been set up in the configuration", , )

ERS_DECLARE_ISSUE_BASE(
    confmodel, NoControlServiceDefined, ConfigurationError,
    "The control service has not been set up for the application " + app_name +
        " you need to define a service called " + app_name + "_control",
    , ((std::string)app_name))


ERS_DECLARE_ISSUE_BASE(
    confmodel, DuplicatedControlService, ConfigurationError,
    "The control service has been defined multiple times for the application " +
        app_name + " you need to define only one service called control",
    , ((std::string)app_name)

)

ERS_DECLARE_ISSUE_BASE(confmodel, BadConf, ConfigurationError,
                       what << where,
                       ,
                       ((std::string)what) ((std::string)where))

ERS_DECLARE_ISSUE_BASE(confmodel, MissingConstructor, ConfigurationError,
    "Configuration contains a " + base + " derived object " + obj
    + " which cannot be cast to " + base 
    + " due to the constructor for the class not being available in this application",
    ,  ((std::string) base)((std::string) obj))

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

ERS_DECLARE_ISSUE_BASE(
    confmodel,
    ReadMaxAllowedIterations,
    AlgorithmError,
    "Has exceeded the maximum of iterations allowed (" << limit << ") during calculation of disabled objects",
    ,
    ((unsigned int)limit))


} // namespace dunedaq


#endif // CONFMODELISSUES_HPP
