require("modules/rttlib")
require("modules/uunit")

rtt.Logger.setlevel("Warning")
var = rtt.Variable
d=TC:getPeer("deployer")

function test_loadlib()
   return d:call("loadLibrary", var.new("string", "lua/testing/testcomp-gnulinux"))
end

function test_create_testcomp()
   if not d:call("loadComponent", var.new("string", "testcomp"), var.new("string", "OCL::Testcomp")) then
      return false
   end
   testcomp = d:getPeer("testcomp")
   return testcomp:getName() == "testcomp"
end

function test_call_op_null_0() return testcomp:call("null_0") end
function test_send_op_null_0() return testcomp:send("null_0"):collect() == 'SendSuccess' end

function test_call_op_0_ct() return testcomp:call("op_0_ct") end
function test_call_op_0_ot() return testcomp:call("op_0_ot") end


local tests = {
   { tstr='return TC:getName() == "lua"' },
   { tstr='return TC:getState() == "PreOperational"' },
   { tstr='return TC:getPeer("deployer") ~= nil' },
   { tstr='return TC:getPeer("gargoyle22") == nil' },
   { tstr='return (TC:getPeer("deployer")):getName() == "deployer"' },
   { tfunc=test_loadlib, descr="trying to load library testcomp-gnulinux" },
   { tfunc=test_create_testcomp, descr="trying to instantiate testcomp" },
   { tfunc=test_call_op_null_0, descr="calling testcomp operation null_0" },
   { tfunc=test_send_op_null_0, descr="sending testcomp operation null_0" },
   { tfunc=test_call_op_0_ct, descr="calling testcomp operation op_0_ct, client thread" },
   { tfunc=test_call_op_0_ot, descr="calling testcomp operation op_0_ot, own thread" },
}


uunit.run_tests(tests, true)
uunit.print_stats(tests)
