--
-- micro unittests
--

require("rttlib")
require("utils")
require("ansicolors")

local rttlib=rttlib
local utils=utils
local col=ansicolors
local loadstring=loadstring
local tostring=tostring

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
      if func == nil then
	 error("loadstring failed: ", err)
	 return nil
      end
   else func = t.tfunc end
   return func()
end

-- execute all tests, 'verb' is boolean verbose flag
function run_tests(tests, verb)
   for i = 1,#tests do
      local t = tests[i]
      t.result = run_test(t)
      if verb then out(cyan(rpad(tostring(i) .. '.', 4)) .. rpad(cyan(t.tstr or t.descr), 73, ' ') , t.result and green("OK", true) or red("FAILED", true)) end
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
