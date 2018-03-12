
#include "FileReporting.hpp"
#include <rtt/Logger.hpp>
#include <rtt/RTT.hpp>
#include "NiceHeaderMarshaller.hpp"
#include "TableMarshaller.hpp"

#include "ocl/Component.hpp"
ORO_LIST_COMPONENT_TYPE(OCL::FileReporting)

namespace OCL {
using namespace RTT;
using namespace std;

FileReporting::FileReporting(const std::string& fr_name)
    : ReportingComponent(fr_name) {}

bool FileReporting::startHook() {
  this->addMarshaller(0, new RTT::TableMarshaller());
  return ReportingComponent::startHook();
}

void FileReporting::stopHook() {
  ReportingComponent::stopHook();
  this->removeMarshallers();
}

bool FileReporting::screenComponent(const std::string& comp) {
  Logger::In in("FileReporting::screenComponent");
  ofstream file((comp + ".screen").c_str());
  if (!file) return false;
  return this->screenImpl(comp, file);
}
}
