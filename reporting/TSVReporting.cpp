#include <rtt/Logger.hpp>
#include <rtt/RTT.hpp>

#include "TSVReporting.hpp"
#include "TSVMarshaller.hpp"

#include "ocl/Component.hpp"
ORO_LIST_COMPONENT_TYPE(OCL::TSVReporting)

namespace OCL {
using namespace RTT;
using namespace std;

TSVReporting::TSVReporting(const std::string& fr_name)
    : ReportingComponent(fr_name) {
    this->onlyNewData = true;
}

bool TSVReporting::startHook() {
  this->addMarshaller(0, new RTT::TSVMarshaller());
  return ReportingComponent::startHook();
}

void TSVReporting::stopHook() {
  ReportingComponent::stopHook();
  this->removeMarshallers();
}

bool TSVReporting::screenComponent(const std::string& comp) {
  Logger::In in("TSVReporting::screenComponent");
  ofstream file((comp + ".screen").c_str());
  if (!file) return false;
  return this->screenImpl(comp, file);
}
}
