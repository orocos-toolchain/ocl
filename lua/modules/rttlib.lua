--
-- Lua-RTT bindings: rttlib convenience module
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

require("utils")
require("ansicolors")

local print, type, table, getmetatable, pairs, ipairs, tostring, assert, error =
   print, type, table, getmetatable, pairs, ipairs, tostring, assert, error
local string = string
local utils = utils
local col = ansicolors
local rtt = rtt
local debug = debug

module("rttlib")

color=false

local function red(str, bright)
   if color then
      str = col.red(str)
      if bright then str=col.bright(str) end
   end
   return str
end

local function blue(str, bright)
   if color then
      str = col.blue(str)
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

local function yellow(str, bright)
   if color then
      str = col.yellow(str)
      if bright then str=col.bright(str) end
   end
   return str
end

local function magenta(str, bright)
   if color then
      str = col.magenta(str)
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

local function white(str, bright)
   if color then
      str = col.white(str)
      if bright then str=col.bright(str) end
   end
   return str
end

-- pretty print a ConnPolicy
function ConnPolicy2tab(cp)
   if cp.type == 0 then cp.type = "DATA"
   elseif cp.type == 1 then cp.type = "BUFFER"
   else cp.type = tostring(cp.type) .. " (invalid!)" end

   if cp.lock_policy == 0 then cp.lock_policy = "UNSYNC"
   elseif cp.lock_policy == 1 then cp.lock_policy = "LOCKED"
   elseif cp.lock_policy == 2 then cp.lock_policy = "LOCK_FREE"
   else cp.lock_policy = tostring(cp.lock_policy) .. " (invalid!)" end

   return cp
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
	 -- detect arrays
	 if #parts == 2 and
	    utils.table_has(parts, "size") and utils.table_has(parts, "capacity") then
	    for i=0,var.size:tolua()-1 do
	       res[i+1] = __var2tab(var[i])
	    end
	 else
	    for i,p in pairs(parts) do
	       res[p] = __var2tab(var:getMember(p))
	    end
	 end
      end
      return res
   end

   return __var2tab(var)
end

-- table of Variable pretty printers
-- must accept a table and return a table or string
var_pp = {}
var_pp.ConnPolicy = ConnPolicy2tab

function var2str(var)
   local res = var2tab(var)

   -- post-beautification:
   local pp = var_pp[var:getType()]
   if pp then
      assert(type(pp) == 'function')
      res = pp(res)
   end

   if type(res) == 'table' then
      return utils.tab2str(res)
   else
      return tostring(res)
   end
end

--
-- update contents of var from a table
--
function varfromtab(var, tab)
   local memdsb
   if type(tab) ~= 'table' then error("arg 2 is not a table") end

   for k,v in pairs(tab) do
      memdsb = var:getMember(k)
      if memdsb == nil then error("no member " .. k) end

      if var_is_basic(memdsb) then
	 memdsb:assign(v);
      else
	 fromtab(memdsb, v)
      end
   end
end


--
-- pretty print properties
--
function prop2str(p)
   return white(p:getName()) .. ' (' .. p:get():getType() .. ')' .. " = " .. yellow(var2str(p:get())) .. red(" // " .. p:getDescription()) .. ""
end

--
-- convert a method to a string
--
function __op2str(name, descr, rettype, arity, args)
   local str = ""

   if #args < 1  then
      str = rettype .. " " .. cyan(name, false) .. "()"
   else
      str = rettype .. " " .. cyan(name, false) .. "("

      for i=1,#args-1 do
	 str = str .. args[i]["type"] .. " " .. args[i]["name"] .. ", "
      end

      str = str .. args[#args]["type"] .. " " .. args[#args]["name"] .. ")"
   end
   if descr then str = str .. " " .. red("// " .. descr) .. "" end
   return str
end

function op2str(op)
   return __op2str(op:info())
end

function tc_op2str(tc, op)
   local rettype, arity, descr, args = tc:getOpInfo(op)
   local str = ""

   if #args < 1  then
      str = rettype .. " " .. cyan(op, false) .. "()"
   else
      str = rettype .. " " .. cyan(op, false) .. "("

      for i=1,#args-1 do
	 str = str .. args[i]["type"] .. " " .. args[i]["name"] .. ", "
      end

      str = str .. args[#args]["type"] .. " " .. args[#args]["name"] .. ")"
   end
   if descr then str = str .. " " .. red("// " .. descr) .. "" end
   return str
end

function service2str(s, inds, indn)
   local inds = inds or '    '
   local indn = indn or 0
   local t = {}

   local function __s2str(s, inds, indn)
      local ind = string.rep(inds, indn)
      t[#t+1] = ind .. magenta("Service: ") .. cyan(s:getName())
      t[#t+1] = ind .. magenta("   Subservices: ") .. cyan(table.concat(s:getProviderNames(), ', '))
      t[#t+1] = ind .. magenta("   Operations:  ") .. cyan(table.concat(s:getOperationNames(), ', '))
      t[#t+1] = ind .. magenta("   Ports:       ") .. cyan(table.concat(s:getPortNames(), ', '))

      utils.foreach(function (sstr)
		       local nexts = s:provides(sstr)
		       __s2str(nexts, inds, indn+1)
		    end, s:getProviderNames())
   end

   __s2str(s, inds, indn)
   return table.concat(t, '\n')
end

function service_req2str(sr, inds, indn)
   local inds = inds or '    '
   local indn = indn or 0
   local t = {}

   local function __sr2str(sr, inds, indn)
      local ind = string.rep(inds, indn)
      t[#t+1] = ind .. magenta("ServiceRequester: ") .. cyan(sr:getRequestName())
      t[#t+1] = ind .. magenta("   Required subservices: ") .. cyan(table.concat(sr:getRequesterNames(), ', '))

      utils.foreach(function (sstr)
		       local nexts = sr:requires(sstr)
		       __sr2str(nexts, inds, indn+1)
		    end, sr:getRequesterNames())
   end

   __sr2str(sr, inds, indn)
   return table.concat(t, '\n')
end


function port2str(p)
   local inf = p:info()
   local ret = {}

   ret[#ret+1] = white(inf.name)
   -- ret[#ret+1] = ' (' .. inf.type .. ')'
   ret[#ret+1] = " ["

   local attrs = {}
   if inf.porttype == 'in' then attrs[#attrs+1] = cyan(inf.porttype, true)
   else attrs[#attrs+1] = magenta(inf.porttype, true) end

   attrs[#attrs+1] = inf.type

   if inf.connected then attrs[#attrs+1] = green("conn")
   else attrs[#attrs+1] = yellow("unconn") end

   if inf.isLocal then attrs[#attrs+1] = "local"
   else  attrs[#attrs+1] = "nonlocal" end

   ret [#ret+1] = table.concat(attrs, ', ')

   ret[#ret+1] = "] "
   ret[#ret+1] = red("// " .. inf.desc)
   return table.concat(ret, '')
end

local function tc_colorstate(state)
   if state == "Init" then return yellow(state, false)
   elseif state == "PreOperational" then return yellow(state, false)
   elseif state == "Running" then return green(state, false)
   elseif state == "Stopped" then return red(state, false) end
   return red(state, true)
end

--
-- pretty print a taskcontext
--
function tc2str(tc)
   local res = {}
   res[#res+1] = magenta('TaskContext') .. ': ' .. green(tc:getName(), true)
   res[#res+1] = magenta("state") .. ": " .. tc_colorstate(tc:getState())

   for i,v in ipairs( { "isActive", "getPeriod" } )
   do
      res[#res+1] = magenta(v) .. ": " .. tostring(tc:call(v)) .. ""
   end

   res[#res+1] = magenta("peers") .. ": " .. table.concat(tc:getPeers(), ', ')
   res[#res+1] = magenta("ports") .. ": "
   for i,p in ipairs(tc:getPortNames(p)) do
      res[#res+1] = "   " .. port2str(tc:getPort(p))
   end

   res[#res+1] = magenta("properties") .. ":"
   for i,p in ipairs(tc:getProperties()) do
      res[#res+1] = "   " .. prop2str(p)
   end

   res[#res+1] = magenta("operations") .. ":"
   for i,v in ipairs(tc:getOps()) do
      res[#res+1] = "   " .. tc_op2str(tc, v)
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
   reg.Variable.fromtab=varfromtab
   reg.Variable.var2tab=var2tab
   reg.Property.__tostring=prop2str
   reg.Service.__tostring=service2str
   reg.ServiceRequester.__tostring=service_req2str
   reg.Operation.__tostring=op2str
   reg.InputPort.__tostring=port2str
   reg.OutputPort.__tostring=port2str
else
   print("no debug library, if required pretty printing must be enabled manually")
end

function info()
   print("services: ", table.concat(rtt.services(), ', '))
   print("typekits: ", table.concat(rtt.typekits(), ', '))
   print("types:    ", table.concat(rtt.types(), ', '))
end

function portval2str(port, comp)
   local inf = port:info()
   local res = white(inf.name) .. ' (' .. inf.type .. ')  ='

   if inf.type == 'unknown_t' then
      res = res .. " ?"
   elseif inf.porttype == 'in' then
      local fs, data = port:read()

      if fs == 'NoData' then res = res .. ' NoData'
      elseif fs == 'NewData' then res = res .. green(var2str(data))
      else res = res .. ' ' .. yellow(var2str(data)) end
   else
      res = res .. ' ' .. cyan(var2str(comp:provides(inf.name):getOperation("last")()))
   end
   return res
end

function portstats(comp)
   for i,p in ipairs(comp:getPortNames(p)) do
      print(portval2str(comp:getPort(p), comp))
   end
end