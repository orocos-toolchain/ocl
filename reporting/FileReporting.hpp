#ifndef ORO_COMP_FILE_REPORTING_HPP
#define ORO_COMP_FILE_REPORTING_HPP

#include <fstream>
#include "ReportingComponent.hpp"

#include <ocl/OCL.hpp>

namespace OCL {
/**
 * A component which writes data reports to a file.
 */
class FileReporting : public ReportingComponent {
 protected:
 public:
  FileReporting(const std::string& fr_name);

  bool startHook();

  void stopHook();

  /**
   * Writes the interface status of \a comp to
   * a file 'comp.screen'.
   */
  bool screenComponent(const std::string& comp);
};
}

#endif
