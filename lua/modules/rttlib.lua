require("utils")
require("ansicolors")

local print, type, table, getmetatable, pairs, ipairs, tostring, assert = print, type, table, getmetatable, pairs, ipairs, tostring, assert
local string = string
local utils = utils
local color = ansicolors
local rtt = rtt
local debug = debug

module("rttlib")

-- pretty print a ConnPolicy
function ConnPolicy2tab(cp)
   local tab = {}
   if cp.type == 0 then tab.type = "DATA"
   elseif cp.type == 1 then tab.type = "DATA"
   else tab.type = "unknown" end

   if cp.lock_policy == 0 then tab.lock_policy = "UNSYNC"
   elseif cp.lock_policy == 1 then tab.lock_policy = "LOCKED"
   elseif cp.lock_policy == 2 then tab.lock_policy = "LOCK_FREE"
   else tab.lock_policy = "unknown" end

   tab.init = cp.init
   tab.pull = cp.pull
   tab.size = cp.size
   tab.transport = cp.transport
   tab.data_size = cp.data_size
   tab.name_id = cp.name_id

   return tab
end

local function var_is_basic(var)
   assert(type(var) == 'userdata', "var_is_basic not a variable:" .. type(var))
   local t = var:getType()
   if t == "bool" or t == "char" or t == "double" or t == "float" or
      t == "int" or t == "string" or t == "uint" or t == "void" then
      return true
   else
      return false
   end
end

function var2tab(var)
   local function __var2tab(var)
      local res
      if var_is_basic(var) then
	 res = var:tolua()
      else
	 local parts = var:getMemberNames()
	 res = {}
	 for i,p in pairs(parts) do
	    res[p] = __var2tab(var:getMember(p))
	 end
      end
      return res
   end

   return __var2tab(var)
end

function var2str(var)
   local res = var2tab(var)

   -- post-beautification
   if var:getType() == 'ConnPolicy' then res = ConnPolicy2tab(res) end

   if type(res) == 'table' then
      return utils.tab2str(res)
   else
      return tostring(res)
   end
end

--
-- pretty print properties
--
function prop2str(p)
   return p:getName() .. '(' .. p:get():getType() .. ')' .. " = " .. var2str(p:get()) .. ' (' .. p:getDescription() .. ')'
end

--
-- convert a method to a string
--
function op2str(tc, op)
   local rettype, arity, args = tc:getOpInfo(op)

   local str = ""

   if #args < 1  then
      str = rettype .. " " .. op .. "()"
   else
      str = rettype .. " " .. op .. "("

      for i=1,#args-1 do
	 str = str .. args[i]["type"] .. " " .. args[i]["name"] .. ", "
      end

      str = str .. args[#args]["type"] .. " " .. args[#args]["name"] .. ")"
   end
   return str
end

--
-- pretty print a taskcontext
--
function tc2str(tc)
   local res = {}
   res[#res+1] = '--- TaskContext "' .. tc:getName() .. '" ---'
   res[#res+1] = "state: " .. tc:getState()

   for i,v in ipairs( { "isActive", "getPeriod" } )
   do
      res[#res+1] = v .. ": " .. tostring(tc:call(v)) .. ""
   end

   res[#res+1] = "peers: " .. table.concat(tc:getPeers(), ', ')
   res[#res+1] = "ports: " .. table.concat(tc:getPortNames(), ', ')

   res[#res+1] = "properties:"
   for i,p in ipairs(tc:getProperties()) do
      res[#res+1] = "   " .. prop2str(p)
   end

   res[#res+1] = "operations:"
   for i,v in ipairs(tc:getOps()) do
      res[#res+1] = "   " .. op2str(tc, v)
   end
   return table.concat(res, '\n')
end

function pptc(tc)
   print(tc2str(tc))
end

-- enable pretty printing
if type(debug) == 'table' then
   reg = debug.getregistry()
   reg.TaskContext.__tostring=tc2str
   reg.Variable.__tostring=var2str
   reg.Property.__tostring=prop2str
else
   print("no debug library, if required pretty printing must be enabled manually")
end
