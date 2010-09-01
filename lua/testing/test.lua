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

function test_call_op_null_0() return testcomp:call("null_0") == nil end
function test_send_op_null_0() return testcomp:send("null_0"):collect() == 'SendSuccess' end

function test_call_op_0_ct() return testcomp:call("op_0_ct") end
function test_call_op_0_ot() return testcomp:call("op_0_ot") end

function test_call_op_2()
   local dbl = var.new("double", 1.1)
   local s = var.new("string", "hello op2")
   local res = testcomp:call("op_2", s, dbl)

   if not res == var.new("double", 2.2) then
      print("wrong result, expected 2.2, got ", res)
      return false
   else
      return true
   end
end

-- test
function test_call_op_1_out()
   local i = var.new("int", 1)
   local res = testcomp:call("op_1_out", i)
   print("result testcomp:call('op_1_out', i): ", i)
   print("result testcomp:call('op_1_out', i): ", i)
   if i ~= var.new("int", 2) then
      print("wrong i, expected 2, got ", i)
      return false
   else
      return true
   end
end

function test_call_op_3_out()
   local s = var.new("string", "hello op3")
   local d = var.new("double", 1.1)
   local i = var.new("int", 33)
   local res=testcomp:call("op_3_out", s, d, i)

   if s ~= var.new("string", "hello op3-this-string-has-a-tail") or d ~= var.new("double", 2.2) or i ~= var.new("int", 4711) then
      print("Checkpoint 1: wrong state of outvalues", s, d, i)
      return false
   end

   print("return value: ", res)

   if s ~= var.new("string", "hello op3-this-string-has-a-tail") or d ~= var.new("double", 2.2) or i ~= var.new("int", 4711) then
      print("Checkpoint 2: wrong state of outvalues", s, d, i)
      return false
   end
   return true
end

function test_call_op_1_out_retval()
   local i = var.new("int", 33)
   local res = testcomp:call("op_1_out_retval", i)

   if i ~= var.new("int", 34) then
      print("Checkpoint 1: wrong i, expected 34, got ", i)
      return false
   end

   print("retval", res)
   print("retval", res)
   print("retval", res)

   if i ~= var.new("int", 34) then
      print("Checkpoint 2: wrong i, expected 34, got ", i)
      return false
   end
   return true
end

function test_var_assignment()
   local i1= var.new("int", 2)
   local i2= var.new("int", 99)

   if i1 == i2 then
      print("comparison error")
      return false
   end

   i1:assign(i2)

   if i1 ~= i2 then
      print("assigment failed")
      return false
   end
   return true
end

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
   { tfunc=test_call_op_2, descr="testcomp:call('op_2', 'hello op2', 1.1)==2.2" },
   { tfunc=test_call_op_1_out, descr="post(testcomp:call('op_1_out', 1)), i==2" },
   { tfunc=test_call_op_3_out, descr="postconditions of testcomp:call('op_3_out', 1)" },
   { tfunc=test_call_op_1_out_retval, descr="post(testcomp:call('op_1_out_retval', 33)), i==34" },
   { tfunc=test_var_assignment, descr="testing assigment of variables" },
}

uunit.run_tests(tests, true)
uunit.print_stats(tests)
