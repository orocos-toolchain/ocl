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

local print, type, table, getmetatable, setmetatable, pairs, ipairs, tostring, assert, error, unpack =
   print, type, table, getmetatable, setmetatable, pairs, ipairs, tostring, assert, error, unpack

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

--- Pretty-print a ConnPolicy.
function ConnPolicy2tab(cp)
   if cp.type == 0 then cp.type = "DATA"
   elseif cp.type == 1 then cp.type = "BUFFER"
   else cp.type = tostring(cp.type) .. " (invalid!)" end

   if cp.lock_policy == 0 then cp.lock_policy = "UNSYNC"
   elseif cp.lock_policy == 1 then cp.lock_policy = "LOCKED"
   elseif cp.lock_policy == 2 then cp.lock_policy = "LOCK_FREE"
   else cp.lock_policy = tostring(cp.lock_policy) .. " (invalid!)" end

   if cp.transport == 0 then cp.transport = "default"
   elseif cp.transport == 1 then cp.transport = "CORBA"
   elseif cp.transport == 2 then cp.transport = "MQUEUE"
   elseif cp.transport == 3 then cp.transport = "ROS"
   else cp.transport = "(invalid)" end
   return cp
end


function var2tab(var)
   local function __var2tab(var)
      local res
      if type(var) ~= 'userdata' then
	 res=var
      elseif rtt.Variable.isbasic(var) then
	 res = var:tolua()
      else -- non basic type
	 local parts = var:getMemberNames()
	 if #parts == 2 and -- catch arrays
	    utils.table_has(parts, "size") and utils.table_has(parts, "capacity") then
	    res = {}
	    if not var.size then return end -- todo: how is this possible?
	    for i=0,var.size-1 do
	       res[i+1] = __var2tab(var[i])
	    end
	 else -- not basic, not array
	    if #parts == 0 then -- no member names -> probably an unrecognized basic type
	       res = var:toString()
	    else -- not basic but with member names
	       res = {}
	       for i,p in pairs(parts) do
		  res[p] = __var2tab(var:getMember(p))
	       end
	    end
	 end
      end
      return res
   end

   return __var2tab(var)
end

--- Table of Variable pretty printing functions.
-- The contents of the table are key=value pairs, the key being the
-- Variable type and value a function which accepts a table as an
-- argument and returns either a table or string.
-- @table var_pp
var_pp = {}
var_pp.ConnPolicy = ConnPolicy2tab

function var2str(var)
   if type(var) ~= 'userdata' then return tostring(var) end

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

--- Update contents of a Variable from a table.
-- Available as a method for Variables using var:fromtab(tab)
-- @param var Variable to update
-- @param tab
function varfromtab(var, tab)
   local memdsb
   if type(tab) ~= 'table' then error("arg 2 is not a table") end

   for k,v in pairs(tab) do
      if type(k) == 'number' then memdsb = var:getMemberRaw(k-1)
      else memdsb = var:getMemberRaw(k) end

      if memdsb == nil then error("no member " .. k) end

      if rtt.Variable.isbasic(memdsb) then
	 memdsb:assign(v);
      else
	 varfromtab(memdsb, v)
      end
   end
end

--- Update contents of a Property from a table.
-- Available as a method for Property using prop:fromtab(tab)
-- @param var Variable to update
-- @param tab appropriate table
function propfromtab(prop, tab)
   return prop:get():fromtab(tab)
end

--- Convert RTT Vector to Lua table
-- @param sv Vector variable
-- @return Lua table
function vect2tab(sv)
   local res = {}
   assert(sv.size and sv.capacity, "vect2tab: arg not a vector (no size or capacity)")
   for i=0,sv.size-1 do res[#res+1] = sv[i] end
   return res
end

--- pretty print properties
-- @param p property
-- @return string
function prop2str(p)
   local info = p:info()
   return white(info.name) .. ' (' .. info.type .. ')' .. " = " .. yellow(var2str(p:get())) .. red(" // " .. info.desc) .. ""
end

--- Convert an operation to a string.
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

--- Taskcontext operation to string.
-- Old version. Using the op2str and __op2str versions are preferred.
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

-- port contents
function portval2str(port, comp)
   local inf = port:info()
   local res = white(inf.name) .. ' (' .. inf.type .. ')  ='

   if inf.type == 'unknown_t' then
      res = res .. " ?"
   elseif inf.porttype == 'in' then
      local fs, data = port:read()

      if fs == 'NoData' then res = res .. ' NoData'
      elseif fs == 'NewData' then res = res .. ' ' .. green(var2str(data))
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
   for i,p in ipairs(tc:getPortNames()) do
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

--- Create an inverse, connected port of a given port.
-- The default name will be the same as the given port.
-- @param p port to create inverse, connected port.
-- @param suffix string to append name of new port (optional)
-- @param cname alternative name for port (optional).
function port_clone_conn(p, suffix, cname)
   local cname = cname or ""
   local suf = suffix or ""
   local inf = p:info()
   local cl
   if inf.porttype == 'in' then
      cl = rtt.OutputPort.new(inf.type, inf.name .. suf, "Inverse port of " .. cname .. "." .. inf.name)
   elseif inf.porttype == 'out' then
      cl = rtt.InputPort.new(inf.type, inf.name .. suf, "Inverse port of " .. cname .. "." .. inf.name)
   else
      error("unkown port type: " .. utils.tab2str(inf))
   end
   if not p:connect(cl) then
      error("ERROR: failed to connect \n\t" .. tostring(p) .. " to its inverse\n\t" .. tostring(cl))
   end
   return cl
end

--- Mirror a TaskContext's connected ports
-- @param comp taskcontext to mirror
-- @param tab table of port names to mirror (default: all)
-- @param suffix suffix to default
-- a table of { port, name, desc } tables
function mirror(comp, suffix, tab)
   local tab = tab or comp:getPortNames()
   local res = {}
   for _,pn in ipairs(tab) do
      local p = comp:getPort(pn)
      local cl = port_clone_conn(p, "_inv", comp:getName())
      res[pn] = cl
   end
   return res
end

--- Find a peer called name
-- Will search through all reachable peers
-- @param name name of component to find
-- @param start_tc optional taskcontext to start search with
function findpeer(name, start_tc)
   local cache = {}
   local function __findpeer(tc)
      local tc_name = tc:getName()
      if cache[tc_name] then return false else cache[tc_name] = true end
      local peers = tc:getPeers()
      if utils.table_has(peers, name) then return tc:getPeer(name) end
      for i,pstr in ipairs(peers) do
	 local p = __findpeer(tc:getPeer(pstr))
	 if p then return p end
      end
      return false
   end
   local start_tc = start_tc or rtt.getTC()
   return __findpeer(start_tc)
end

--- Call func on all reachable peers and return results a flattened table
-- @param func function to call on peer
-- @param start_tc optional taskcontext to start search with. If none given the current will be used.
function mappeers(func, start_tc)
   local cache = {}
   local res = {}
   local function __mappeers(tc)
      local tc_name = tc:getName()
      if cache[tc_name] then return else cache[tc_name] = true end
      res[tc_name] = func(tc)
      for i,pn in ipairs(tc:getPeers()) do __mappeers(tc:getPeer(pn)) end
   end
   local start_tc = start_tc or rtt.getTC()
   __mappeers(start_tc)
   return res
end

--- Print useful information
-- print information about availabe services, typekits, types and
-- component types
function info()
   local ind="            "
   local ind1=""
   print(magenta("services:   ") .. utils.wrap(table.concat(rtt.services(), ' '), 80, ind, ind1))
   print(magenta("typekits:   ") .. utils.wrap(table.concat(rtt.typekits(), ' '), 80, ind, ind1))
   print(magenta("types:      ") .. utils.wrap(table.concat(rtt.types(), ' '), 80, ind, ind1))

   local depl = findpeer("deployer")
   if depl and rtt.TaskContext.hasOperation(depl, "getComponentTypes") then
      local t = var2tab(depl:getComponentTypes())
      -- print(magenta("comp types: "), table.concat(t, ', '))
      print(magenta("comp types: ") .. utils.wrap(table.concat(t, ' '), 80, ind, ind1))
   end
end

--- Check if a typekit has been loaded.
function typekit_loaded(n) return utils.table_has(rtt.typekits(), n) end

--- Enable Service:op() syntax
-- Service metatable __index replacement for allowing operations
-- to be called like methods. This is fairly slow, use getOperation to
-- cache local op when speed matters.
function service_index(srv, key)
   local reg = debug.getregistry()
   if rtt.Service.hasOperation(srv, key) then
      return function (srv, ...)
		local op = rtt.Service.getOperation(srv, key)
		return op(...)
	     end
   else -- pass on to standard metatable
      return reg.Service[key]
   end
end

--- Enable tc:op() syntax
-- TaskContext metatable __index replacement for allowing operations
-- to be called like methods. This is pretty slow, use getOperation to
-- cache local op when speed matters.
function tc_index(tc, key)
   local reg = debug.getregistry()
   if rtt.TaskContext.hasOperation(tc, key) then
      return function (tc, ...) return rtt.TaskContext.call(tc, key, ...) end
   else -- pass on to standard metatable
      return reg.TaskContext[key]
   end
end

setmetatable(rtt.Variable, {__call=function(t,...) return rtt.Variable.new(...) end})
setmetatable(rtt.Property, {__call=function(t,...) return rtt.Property.new(...) end})
setmetatable(rtt.InputPort, {__call=function(t,...) return rtt.InputPort.new(...) end})
setmetatable(rtt.OutputPort, {__call=function(t,...) return rtt.OutputPort.new(...) end})
setmetatable(rtt.EEHook, {__call=function(t,...) return rtt.EEHook.new(...) end})

-- enable pretty printing
if type(debug) == 'table' then
   reg = debug.getregistry()
   reg.TaskContext.__tostring=tc2str
   reg.TaskContext.stat=portstats
   reg.Service.__index=service_index -- enable operations as methods
   reg.TaskContext.__index=tc_index -- enable operations as methods
   reg.Variable.__tostring=var2str
   reg.Variable.fromtab=varfromtab
   reg.Variable.var2tab=var2tab
   reg.Property.__tostring=prop2str
   reg.Property.fromtab=propfromtab
   reg.Service.__tostring=service2str
   reg.ServiceRequester.__tostring=service_req2str
   reg.Operation.__tostring=op2str
   reg.InputPort.__tostring=port2str
   reg.OutputPort.__tostring=port2str
else
   print("no debug library, if required pretty printing must be enabled manually")
end
