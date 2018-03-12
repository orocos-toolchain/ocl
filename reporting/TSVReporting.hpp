#ifndef ORO_COMP_TSV_REPORTING_HPP
#define ORO_COMP_TSV_REPORTING_HPP

#include <fstream>
#include "ReportingComponent.hpp"

#include <ocl/OCL.hpp>

namespace OCL {

/**
 * @brief The TSVReporting class writes the reported values to a collection of
 * TSV (Tab Separated Values) files.
 * Each reported port ends up in its own TSV file, named after the port. This
 * solves the problem of ports, that emit data on different frequencies. It also
 * makes any header writing obsolete, enabling us to add new reported ports
 * during runtime. By default reportOnlyNewData is set to true.
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
