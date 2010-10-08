#!/usr/bin/env rttlua

-- Lua-RTT bindings
--
-- (C) Copyright 2010 Markus Klotzbuecher,
-- markus.klotzbuecher@mech.kuleuven.be,
-- Department of Mechanical Engineering,
-- Katholieke Universiteit Leuven, Belgium.
--
-- This library is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public
-- License as published by the Free Software Foundation;
-- version 2 of the License.
--
-- As a special exception, you may use this file as part of a free
-- software library without restriction.  Specifically, if other files
-- instantiate templates or use macros or inline functions from this
-- file, or you compile this file and link it with other files to
-- produce an executable, this file does not by itself cause the
-- resulting executable to be covered by the GNU General Public
-- License.  This exception does not however invalidate any other
-- reasons why the executable file might be covered by the GNU General
-- Public License.
--
-- This library is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- Lesser General Public License for more details.
--
-- You should have received a copy of the GNU General Public
-- License along with this library; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place,
-- Suite 330, Boston, MA  02111-1307  USA


require("rttlib")
require("uunit")

rtt.Logger.setlevel("Warning")
var = rtt.Variable
TC=rtt.getTC()
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

function test_coercion()
   local x = testcomp:call("op_2", "a-lua-string", 33.3)
   return x:tolua() == 66.6
end

function test_send_op2()
   local sh = testcomp:send("op_2", "hullo", 55.5)
   ss, res = sh:collect()
   if ss ~= "SendSuccess" or res ~= var.new("double", 111) then
      return false
   else
      return true
   end
end

function test_dataflow_lua()
   po = rtt.OutputPort.new("string", "po", "my output port")
   pi = rtt.InputPort.new("string", "pi", "my input port")
   TC:addPort(po)
   TC:addPort(pi)
   print("connecting ports... ", d:call("connectTwoPorts", "lua", "po", "lua", "pi"))
   po:write("hello_ports")
   local res, val = pi:read()
   return res == "NewData" and val:tolua() == "hello_ports"
end

function test_lua_service()
   -- load lua service into deployer
   d:addPeer(d)
   d:call("loadService", "deployer", "Lua")
   local execstr_op = d:provides("Lua"):getOperation("exec_str")
   execstr_op([[
		    require("rttlib")
		    local tc=rtt.getTC()
		    local p=rtt.Property.new("string", "service-testprop")
		    tc:addProperty(p)
		    p:set("hullo from the lua service!")
	      ]])

   local p = d:getProperty("service-testprop")
   local res =  p:get() == var.new("string", "hullo from the lua service!")
   -- d:removePeer("service-testprop")
   -- p:delete()
   return res
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
   { tfunc=test_coercion, descr="testing coercion of variables in call" },
   { tfunc=test_send_op2, descr="testing send for op_2" },
   { tfunc=test_dataflow_lua, descr="testing dataflow with conversion from/to basic lua types" },
   { tfunc=test_lua_service, descr="test interaction with lua service" },
}

uunit.run_tests(tests, true)
uunit.print_stats(tests)
