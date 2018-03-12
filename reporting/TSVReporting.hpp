#ifndef ORO_COMP_TSV_REPORTING_HPP
#define ORO_COMP_TSV_REPORTING_HPP

#include <fstream>
#include "ReportingComponent.hpp"

#include <ocl/OCL.hpp>

namespace OCL {
/**
 * A component which writes data reports to a file.
 */
class TSVReporting : public ReportingComponent {
 protected:
 public:
  TSVReporting(const std::string& fr_name);

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
