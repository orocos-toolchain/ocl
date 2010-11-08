--
-- micro unittests
--
-- Lua-RTT bindings
--
-- (C) Copyright 2010 Markus Klotzbuecher
-- markus.klotzbuecher@mech.kuleuven.be
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
--

require("rttlib")
require("utils")
require("ansicolors")

local rttlib=rttlib
local utils=utils
local col=ansicolors
local string=string
local loadstring=loadstring
local tostring=tostring
local pcall=pcall

local error = utils.stderr
local out = utils.stdout
local map = utils.map
local rpad = utils.rpad

module("uunit")

color=true
local function red(str, bright)
   if color then
      str = col.red(str)
      if bright then str=col.bright(str) end
   end
   return str
end

local function cyan(str, bright)
   if color then
      str = col.cyan(str)
      if bright then str=col.bright(str) end
   end
   return str
end

local function green(str, bright)
   if color then
      str = col.green(str)
      if bright then str=col.bright(str) end
   end
   return str
end


-- run func, print mes if failed and return status
local function run_test(t)
   local func, err
   if t.tstr then
      func, err = loadstring(t.tstr)
      if func == nil then return false, err end
   else
      func = t.tfunc
   end

   local call_stat, res = pcall(func)
   if call_stat == true then return res
   else return false, res end
   return false, "no test to run found"
end

-- execute all tests, 'verb' is boolean verbose flag
function run_tests(tests, verb)
   local err
   for i = 1,#tests do
      local t = tests[i]
      t.result, err = run_test(t)
      if not t.result and err then out(err) end
      if verb then
	 out(cyan(rpad(tostring(i) .. '.', 4)) .. "Tested " ..
	  rpad(cyan(t.tstr or t.descr), 75, ' ') , t.result and green("OK", true) or red("FAILED", true))
      end
      -- out(string.rep('-', 10))
   end
end

function print_stats(tests)
   local succ, fail = 0, 0
   for i = 1,#tests do
      if tests[i].result then succ = succ + 1
      else fail = fail + 1 end
   end
   out("Run " .. #tests .. " tests. " .. succ .. " succeeded, " .. fail .. " failed.")
end
