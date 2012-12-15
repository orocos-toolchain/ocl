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

rtt.setLogLevel("Warning")
var = rtt.Variable
TC=rtt.getTC()
d=TC:getPeer("Deployer")

function fails() return false end

function test_loadlib()
   return d:import("ocl")
end

--- Test
function test_create_testcomp()
   if not d:loadComponent("testcomp", "OCL::Testcomp") then
      return false
   end
   testcomp = d:getPeer("testcomp")
   return testcomp:getName() == "testcomp"
end

function test_call_op_null_0() return testcomp:null_0() == nil end
function test_send_op_null_0() return testcomp:getOperation("null_0"):send():collect() == 'SendSuccess' end
function test_call_op_0_ct() return testcomp:op_0_ct() end
function test_call_op_0_ot() return testcomp:op_0_ot() end

function test_call_op_2()
   local dbl = var.new("double", 1.1)
   local s = var.new("string", "hello op2")
   local res = testcomp:op_2(s, dbl)

   if not res == 2.2 then
      print("wrong result, expected 2.2, got ", res)
      return false
   else
      return true
   end
end

-- test
function test_call_op_1_out()
   local i = var.new("int", 1)
   local res = testcomp:op_1_out(i)

   if i:tolua() ~= 2 then
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
   local res=testcomp:op_3_out(s, d, i)

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
   local res = testcomp:op_1_out_retval(i)

   if i ~= var.new("int", 34) then
      print("Checkpoint 1: wrong i, expected 34, got ", i)
      return false
   end

   print("retval", res)
   print("retval", res)
   print("retval", i)

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
   local x = testcomp:op_2("a-lua-string", 33.3)
   return x == 66.6
end

function test_send_op2()
   local sh = testcomp:getOperation("op_2"):send("hullo", 55.5)
   ss, res = sh:collect()
   if ss ~= "SendSuccess" or res ~= 111 then
      return false
   else
      return true
   end
end

function test_send_op2_with_collect_args()
   local res=rtt.Variable("double")
   local sh = testcomp:getOperation("op_2"):send("hullo", 55.5)
   ss = sh:collect(res)
   if ss ~= "SendSuccess" or res:tolua() ~= 111 then
      print("SendSucess:", ss, ", res: ", res)
      return false
   else return true end
end


function test_dataflow_lua()
   po = rtt.OutputPort.new("string", "po", "my output port")
   pi = rtt.InputPort.new("string", "pi", "my input port")
   TC:addPort(po)
   TC:addPort(pi)
   print("connecting ports... ", d:connectTwoPorts("lua", "po", "lua", "pi"))
   po:write("hello_ports")
   local res, val = pi:read()
   return res == "NewData" and val == "hello_ports"
end

function test_lua_service()
   -- load lua service into deployer
   d:addPeer("Deployer", "Deployer")
   d:loadService("Deployer", "Lua")
   local execstr_op = d:provides("Lua"):getOperation("exec_str")
   execstr_op([[
		    require("rttlib")
		    local tc=rtt.getTC()
		    local p=rtt.Property.new("string", "service-testprop")
		    tc:addProperty(p)
		    p:set("hullo from the lua service!")
	      ]])

   local p = d:getProperty("service-testprop")
   local res =  p:get() == "hullo from the lua service!"
   d:removeProperty("service-testprop")
   p:delete()
   d:removePeer("Deployer")
   return res
end

--
-- create a LuaComponent and load a Lua Service into it. The lua
-- service then creates registers an EEHook in which in increments a
-- variable. When reaching the number 17 it disables the hook and
-- writes the value 17 to the result property, which then is checked
-- by this testscript to see if everything went ok.
--
function test_lua_eehook()
   d:loadComponent("c_eehook", "OCL::LuaComponent")
   d:loadService("c_eehook", "Lua")
   c_eehook = d:getPeer("c_eehook")

   c_eehook:exec_str('function configureHook() return true end')
   c_eehook:exec_str('function startHook() return true end')
   c_eehook:exec_str('function updateHook() return true end')
   c_eehook:exec_str('function stopHook() return true end')
   c_eehook:exec_str('function cleanupHook() return true end')
   c_eehook:configure()

   d:setActivity("c_eehook", 0.01, 0, 0)

   local execstr_op = c_eehook:provides("Lua"):getOperation("exec_str")
   execstr_op([[
		    require("rttlib")
		    local tc=rtt.getTC()
		    counter = 0
		    local p=rtt.Property.new("int", "result")
		    tc:addProperty(p)
		    eeh = rtt.EEHook.new("foobar")

		    function foobar ()
		       counter = counter + 1
		       if counter >= 17 then
			  p:set(counter)
			  print("disabling hook", eeh:disable())
			  return false
		       end
		       return true
		    end

		    eeh:enable()

	      ]])

   c_eehook:start()
   os.execute("sleep 0.5")
   -- check result
   res_prop = c_eehook:getProperty("result")
   res = res_prop:get() == 17
   return res
end

function call_uint8_arg()
   d:import("rtt_rosnode")
   x=rtt.Variable("uint8", 3)
   return not testcomp:op1_uint8(x) and testcomp:op1_uint8(120)
end

local tests = {
   { tstr='return TC:getName() == "lua"' },
   { tstr='return TC:getState() == "PreOperational"' },
   { tstr='return TC:getPeer("Deployer") ~= nil' },
   { tstr='return (TC:getPeer("Deployer")):getName() == "Deployer"' },
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
   { tfunc=call_uint8_arg, descr="testing an operation call with an uint8 argument" },
   { tfunc=test_var_assignment, descr="testing assigment of variables" },
   { tfunc=test_coercion, descr="testing coercion of variables in call" },
   { tfunc=test_send_op2, descr="testing send for op_2 and collect()" },
   { tfunc=test_send_op2_with_collect_args, descr="testing sending op_2 and collect(var)"},
   { tfunc=test_dataflow_lua, descr="testing dataflow with conversion from/to basic lua types" },
   { tfunc=test_lua_service, descr="testing interaction with lua service" },
   { tfunc=test_lua_eehook, descr="testing EEHook" },
}

uunit.run_tests(tests, true)
uunit.print_stats(tests)
