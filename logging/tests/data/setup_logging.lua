
-- Run this test in your build/logging/tests directory with the command:
-- rttlua-gnulinux -i data/setup_logging.lua

require("rttlib")
 
rtt.setLogLevel("Info")

-- Set this to true to write the property files the first time.
write_props=false
 
tc = rtt.getTC()
depl = tc:getPeer("Deployer")
depl:import(".")
 
-- Create components. Enable BUILD_LOGGING and BUILD_TESTS for this to
-- work.
depl:loadComponent("AppenderA", "OCL::logging::FileAppender")
depl:setActivity("AppenderA", 1, 0, 0)

depl:loadComponent("AppenderB", "OCL::logging::OstreamAppender")
depl:setActivity("AppenderB", 1, 0, 0)
 
depl:loadComponent("LoggingService", "OCL::logging::LoggingService")
depl:setActivity("LoggingService", 0.5, 0, 0)

depl:loadComponent("TestComponent","OCL::logging::test::Component")
depl:setActivity("TestComponent", 0.01, 0, 0)

aa = depl:getPeer("AppenderA")
ab = depl:getPeer("AppenderB")
ls = depl:getPeer("LoggingService")
test = depl:getPeer("TestComponent")
 
depl:addPeer("LoggingService","AppenderA")
depl:addPeer("LoggingService","AppenderB")
 
-- Load marshalling service to read/write components
depl:loadService("LoggingService","marshalling")
depl:loadService("AppenderA","marshalling")
depl:loadService("AppenderB","marshalling")
 
if write_props then
	ls:provides("marshalling"):writeProperties("data/logging_properties.cpf")
	aa:provides("marshalling"):writeProperties("data/appenderA_properties.cpf")
	ab:provides("marshalling"):writeProperties("data/appenderB_properties.cpf")
	print("Wrote property files. Edit them and set write_props=false")
	os.exit(0)
else
	ls:provides("marshalling"):loadProperties("data/logging_properties.cpf")
	aa:provides("marshalling"):loadProperties("data/appenderA_properties.cpf")
	ab:provides("marshalling"):loadProperties("data/appenderB_properties.cpf")
end
 
ls:configure()
-- Test reconfiguration
ls:configure()
ls:start()
ls:logCategories()

aa:configure()
ab:configure()
test:configure()
 
aa:start()
ab:start()
test:start()
