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

local defops_lst = { "activate", "cleanup", "configure", "error", "getCpuAffinity", "getPeriod",
		     "inRunTimeError", "isActive", "isConfigured", "isRunning", "setCpuAffinity",
		     "setPeriod", "start", "stop", "trigger", "update", "inFatalError" }

-- shortcut
local TaskContext=rtt.TaskContext

--- Condition colorization.
--

local function red(str, bright)
   if color then str = col.red(str); if bright then str=col.bright(str) end end
   return str
end

local function blue(str, bright)
   if color then str = col.blue(str); if bright then str=col.bright(str) end end
   return str
end

local function green(str, bright)
   if color then str = col.green(str); if bright then str=col.bright(str) end end
   return str
end

local function yellow(str, bright)
   if color then str = col.yellow(str); if bright then str=col.bright(str) end end
   return str
end

local function magenta(str, bright)
   if color then str = col.magenta(str); if bright then str=col.bright(str) end end
   return str
end

local function cyan(str, bright)
   if color then str = col.cyan(str); if bright then str=col.bright(str) end end
   return str
end

local function white(str, bright)
   if color then str = col.white(str); if bright then str=col.bright(str) end end
   return str
end


--- Indent a string if it contains newlines.
-- Used to nicely print multiline values and keep single line values
-- on one line. Will add a newline to the first line and indent.
-- @param str string to conditionally indent.
-- @param ind indentation
-- @return indented or unmodified string.
function if_nl_ind(str, ind)
   ind = ind or "\t\t"
   if not string.match(str, '\n') then return str end
   return string.gsub('\n'..str, '\n', '\n'..ind)
end

--- Beautify a ConnPolicy table.
-- @param cp ConnPolicy table
-- @return the processed table
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


--- Convert an RTT Variable to a table.
-- @param var Variable
-- @return table tab representation of Variable
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

--- Convert a RTT Variable to a string.
-- @param var
-- @return string
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
      if type(k) == 'number' then
	 if k > var.size then var:resize(k) end
	 memdsb = var:getMemberRaw(k-1)
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
   return white(info.name) .. ' (' .. info.type .. ')' .. " = " .. if_nl_ind(yellow(var2str(p:get()))) .. red(" // " .. info.desc) .. ""
end

--- Convert an operation to a string.
-- @param name name of operation
-- @param descr description
-- @param rettype return type
-- @param arity arity of operation
-- @param args table of argument tables {type, name}
-- @return string
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

--- Convert an operation to a string.
-- @param op Operation
-- @return string
function op2str(op)
   return __op2str(op:info())
end

--- Taskcontext operation to string.
-- Old version. Using the op2str and __op2str versions are preferred.
function tc_op2str(tc, op)
   local rettype, arity, descr, args = TaskContext.getOpInfo(tc, op)
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
      t[#t+1] = ind .. magenta("Service: ") .. cyan(s:getName(), true)
      t[#t+1] = ind .. magenta("   Subservices: ") .. cyan(table.concat(s:getProviderNames(), ', '))

      t[#t+1] = ind .. magenta("   Ports:       ") -- .. cyan(table.concat(s:getPortNames(), ', '))
      utils.foreach(function(portname)
		       t[#t+1] = ind .. "        " .. port2str(s:getPort(portname))
		    end, s:getPortNames())

      t[#t+1] = ind .. magenta("   Properties:  ")
      utils.foreach(function(p)
		       t[#t+1] = ind .. "       " .. prop2str(p)
		    end, s:getProperties())

      t[#t+1] = ind .. magenta("   Operations:  ")
      utils.foreach(function(opname)
		       t[#t+1] = ind .. "        " .. op2str(s:getOperation(opname))
		    end, s:getOperationNames())

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


--- Convert a port to a string.
-- @param p Port
-- @param nodoc don't add documentation
-- @return string
function port2str(p, nodoc)
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
   if not nodoc then ret[#ret+1] = red("// " .. inf.desc) end
   return table.concat(ret, '')
end

--- Convert the value of the port to a coloured string.
-- @param port
-- @param comp component this port belongs to (used to access port service)
-- @param string
function portval2str(port, comp)
   local inf = port:info()
   local portstr = white(inf.name) .. ' (' .. inf.type .. ')  = '
   local value
   if inf.type == 'unknown_t' then value = "?"
   elseif inf.porttype == 'in' then
      local fs, data = port:read()

      if fs == 'NoData' then value=' NoData'
      elseif fs == 'NewData' then value = green(var2str(data))
      else value = yellow(var2str(data)) end
   else
      value = cyan(var2str(comp:provides(inf.name):getOperation("last")()))
   end
   return portstr .. if_nl_ind(value)
end

--- Print the values of all ports of a component.
-- @param comp TaskContext
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

--- Convert a TaskContext to a nice string.
-- @param tc TaskContext
-- @return string
function tc2str(tc, full)
   local res = {}
   res[#res+1] = magenta('TaskContext') .. ': ' .. green(TaskContext.getName(tc), true)
   res[#res+1] = magenta("   state") .. ": " .. tc_colorstate(TaskContext.getState(tc))

   for i,v in ipairs( { "isActive", "getPeriod" } ) do
      res[#res+1] = "   " .. magenta(v) .. ": " .. tostring(TaskContext.getOperation(tc, v)()) .. ""
   end

   res[#res+1] = magenta("   Peers") .. ": " .. cyan(table.concat(TaskContext.getPeers(tc), ', '))
   res[#res+1] = magenta("   Services") .. ": " .. cyan(table.concat(TaskContext.getProviderNames(tc), ', '))
   res[#res+1] = magenta("   Ports") .. ": "
   for i,p in ipairs(TaskContext.getPortNames(tc)) do
      res[#res+1] = "       " .. port2str(TaskContext.getPort(tc, p))
   end

   res[#res+1] = magenta("   Properties") .. ":"
   for i,p in ipairs(TaskContext.getProperties(tc)) do
      res[#res+1] = "      " .. prop2str(p)
   end

   res[#res+1] = magenta("   Operations") .. ":"
   for i,v in ipairs(TaskContext.getOps(tc)) do
      if not utils.table_has(defops_lst, v) or full then
	 res[#res+1] = "      " .. tc_op2str(tc, v)
      end
   end
   return table.concat(res, '\n')
end

--- Print a TaskContext.
function pptc(tc)
   print(tc2str(tc))
end

-- Sample iface data structure for create_if function below.
-- iface={
--    ports={
--       { name='conf_events', datatype='string', type='in+event', desc="Configuration events in-port" },
--       { name='foo', datatype='int', type='in', desc="numeric in-port" },
--       { name='conf_status', datatype='string', type='out', desc="Current configuration status" },
--    },
--    properties={
--       { name='configurations', datatype='string', desc="Set of configuration" },
--    }
-- }

--- Construct a port/property interface from an interface spec.
-- If a port/property with name exists it is left untouched.
-- The interface can be removed by using rttlib.tc_cleanup()
-- @param iface interface specification
-- @param tc optional TaskContext of interface to construct. default is rtt.getTC().
function create_if(iface, tc)
   local tc = tc or rtt.getTC()
   local res={ ports={}, props={} }

   function create_port(pspec, i)
      local p
      assert(pspec.name, "missing port name in entry"..tostring(i))
      pspec.desc=pspec.desc or ""

      -- probably should check if type and event match
      if tc_has_port(tc, pspec.name) then
	 res.ports[pspec.name] = tc:getPort(pspec.name)
	 return
      end

      if pspec.type=='out' then
	 p=rtt.OutputPort(pspec.datatype)
      elseif pspec.type=='in' or pspec.type=='in+event' then
	 p=rtt.InputPort(pspec.datatype)
      else
	 error("unknown port type "..tostring(pspec.type))
      end
      if pspec.type=='in+event' then
	 tc:addEventPort(p, pspec.name, pspec.desc)
      else
	 tc:addPort(p, pspec.name, pspec.desc)
      end
      res.ports[pspec.name]=p
   end

   function create_prop(pspec, i)
      local p
      assert(pspec.name, "missing property name in entry"..tostring(i))
      pspec.desc=pspec.desc or ""

      if tc_has_property(tc, pspec.name) then
	 res.props[pspec.name] = tc:getProperty(pspec.name)
	 return
      end
      p=rtt.Property(pspec.datatype)
      tc:addProperty(p, pspec.name, pspec.desc)
      res.props[pspec.name]=p
   end

   utils.foreach(create_port, iface.ports)
   utils.foreach(create_prop, iface.properties)
   return res
end

--- Cleanup ports and properties of this Lua Component.
-- Only use this function if the proper properties were actually
-- created from Lua (yes, this will be commonly the case).  The
-- built-in Properties 'lua_string' and 'lua_file' are ignored.
-- @return number of removed properties, number of removed ports
function tc_cleanup()
   local tc=rtt.getTC()

   local function cleanup_prop(pname)
      local prop = TaskContext.getProperty(tc, pname)
      TaskContext.removeProperty(tc, pname)
      prop:delete()
   end

   local function cleanup_port(pname)
      local port = TaskContext.getPort(tc, pname)
      TaskContext.removePort(tc, pname)
      port:delete()
   end

   -- get list of property names (remove built-in ones)
   local propnames=utils.filter(function(n,i)
				   if n=='lua_string' or n=='lua_file' then
				      return false
				   end
				   return true
				end, TaskContext.getPropertyNames(tc))

   local portnames = TaskContext.getPortNames(tc)

   utils.foreach(cleanup_prop, propnames)
   utils.foreach(cleanup_port, portnames)
   return #propnames, #portnames
end

--- Check if a TaskContext has a port with name
-- @param tc TaskContext
-- @param name port name to check for
-- @return true or false
function tc_has_port(tc, name)
   return utils.table_has(TaskContext.getPortNames(tc), name)
end

--- Check if a TaskContext has a property with name
-- @param tc TaskContext
-- @param name property name to check for
-- @return true or false
function tc_has_property(tc, name)
   return utils.table_has(TaskContext.getPropertyNames(tc), name)
end


--- Create an inverse, connected port of a given port.
-- The default name will be the same as the given port.
-- @param p port to create inverse, connected port.
-- @param suffix string to append name of new port (optional)
-- @param cname alternative name for port (optional).
function port_clone_conn(p, suffix, cname)
   local inf = p:info()
   local suf = suffix or ""
   local cname = cname or inf.name

   local cl
   if inf.porttype == 'in' then
      cl = rtt.OutputPort.new(inf.type, cname .. suf, "Inverse port of " .. inf.name)
   elseif inf.porttype == 'out' then
      cl = rtt.InputPort.new(inf.type, cname .. suf, "Inverse port of " .. inf.name)
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
      local tc_name = TaskContext.getName(tc)
      if cache[tc_name] then return false else cache[tc_name] = true end
      local peers = TaskContext.getPeers(tc)
      if utils.table_has(peers, name) then return TaskContext.getPeer(tc, name) end
      for i,pstr in ipairs(peers) do
	 local p = __findpeer(TaskContext.getPeer(tc, pstr))
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
      local tc_name = TaskContext.getName(tc)
      if cache[tc_name] then return else cache[tc_name] = true end
      res[tc_name] = func(tc)
      for i,pn in ipairs(TaskContext.getPeers(tc)) do __mappeers(TaskContext.getPeer(tc, pn)) end
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

   local depl = findpeer("Deployer")
   if depl and rtt.TaskContext.hasOperation(depl, "getComponentTypes") then
      local t = var2tab(depl:getComponentTypes())
      -- print(magenta("comp types: "), table.concat(t, ', '))
      print(magenta("comp types: ") .. utils.wrap(table.concat(t, ' '), 80, ind, ind1))
   end
end

function stat()
   function __stat_tc(tc)
      local state = TaskContext.getState(tc)
      print(table.concat{utils.strsetlen(TaskContext.getName(tc), 40, true),
			 tc_colorstate(TaskContext.getState(tc)) .. string.rep(' ', 20-string.len(state)),
			 utils.strsetlen(tostring(tc:isActive()), 10, false),
			 utils.strsetlen(tostring(tc:getPeriod()), 10, false)}, ' ')
   end

   print(table.concat{utils.rpad("Name", 40), utils.rpad("State", 20),
		      utils.rpad("isActive", 10), utils.rpad("Period", 10)}, ' ')
   mappeers(__stat_tc)
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
      return function (tc, ...) return rtt.TaskContext.getOperation(tc, key)(...) end
   else -- pass on to standard metatable
      return reg.TaskContext[key]
   end
end

--- Return the (RTT aware) type of an object.
-- falls back on the standard 'type' if not an RTT object.
-- @param obj
-- @return string
function rtt_type(obj)
   local mt = getmetatable(obj)
   if mt then
      local reg=debug.getregistry()
      for k,v in pairs(reg) do if v == mt then return k end end
   end
   return type(obj)
end

-- conveniance constructors
setmetatable(rtt.Variable, {__call=function(t,...) return rtt.Variable.new(...) end})
setmetatable(rtt.Property, {__call=function(t,...) return rtt.Property.new(...) end})
setmetatable(rtt.InputPort, {__call=function(t,...) return rtt.InputPort.new(...) end})
setmetatable(rtt.OutputPort, {__call=function(t,...) return rtt.OutputPort.new(...) end})
setmetatable(rtt.EEHook, {__call=function(t,...) return rtt.EEHook.new(...) end})

-- create a globals tab with a metatable that forwards __index to
-- globals_get() and __tostring prints all know globals.
rtt.globals={}
local globals_mt= {
   __index=function(t,v) return rtt.globals_get(v) end,
   __tostring=function(t)
		 local res = {}
		 for _,v in ipairs(rtt.globals_getNames()) do res[v] = rtt.globals_get(v) end
		 return utils.tab2str(res)
	      end
}
setmetatable(rtt.globals, globals_mt)

-- enable pretty printing
if type(debug) == 'table' then
   reg = debug.getregistry()
   reg.TaskContext.__tostring=tc2str
   reg.TaskContext.show=function(tc) return tc2str(tc, true) end
   reg.TaskContext.stat=portstats
   reg.Service.__index=service_index -- enable operations as methods
   reg.TaskContext.__index=tc_index -- enable operations as methods
   reg.Variable.__tostring=var2str
   reg.Variable.fromtab=varfromtab
   reg.Variable.var2tab=var2tab
   reg.Property.__tostring=prop2str
   reg.Property.fromtab=propfromtab
   reg.Service.__tostring=service2str
   reg.Service.stat=portstats
   reg.ServiceRequester.__tostring=service_req2str
   reg.Operation.__tostring=op2str
   reg.InputPort.__tostring=port2str
   reg.OutputPort.__tostring=port2str
else
   print("no debug library, if required pretty printing must be enabled manually")
end
