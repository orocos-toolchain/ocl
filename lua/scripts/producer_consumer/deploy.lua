#!/usr/bin/env rttlua

require("rttlib")
require("utils")

-- create components specified in table spec
function create_comps(deployer, spec)
   local res = {}
   for _,v in ipairs(spec) do
      io.write("creating component " .. v.name .. " of type " .. v.type .. "... ")
      print(deployer:call("loadComponent", v.name, v.type))
      res[v.name] = d:getPeer(v.name)
   end
   return res
end

-- create connections specified in table spec
function create_conns(deployer, spec)
   for _,v in ipairs(spec) do
      local cp = rtt.Variable.new("ConnPolicy")
      cp:fromtab(v.connpolicy)
      io.write("creating connection from " .. 
	       v.srccomp .. "." .. v.srcport .. "->" ..
	       v.tgtcomp .. "." .. v.tgtport .. ", ConnPolicy: " .. tostring(cp) .. "... ")
      print(d:call("connectTwoPorts", v.srccomp, v.srcport, v.tgtcomp, v.tgtport))
   end
end

function create_activities(deployer, spec)
   for _,v in ipairs(spec) do
      v.prio = v.prio or 0
      v.schedtype = v.schedtype or 1 -- ORO_SCHED_OTHER by default
      io.write("creating activity for " .. v.name .. ", period=" .. v.period .. 
	       ", prio=" .. v.prio .. ", schedtype=" .. v.schedtype .. "... ")
      print(deployer:call("setActivity", v.name, v.period, v.prio, v.schedtype))
   end
end

tc = rtt.getTC()
d = tc:getPeer("deployer")

d:call("displayComponentTypes")

-- instantiate component
comp_spec = { { type="OCL::LuaComponent", name="Producer" }, 
	      { type="OCL::LuaComponent", name="Consumer" } }

l = create_comps(d, comp_spec)

-- create activities
act_spec = { { name="Producer", period=1, prio=0, schedtype=0 } }

create_activities(d, act_spec)

l.Producer:call("exec_file", "producer.lua")
l.Consumer:call("exec_file", "consumer.lua")

l.Producer:configure()
l.Consumer:configure()

-- create connections
conn_spec = { { srccomp="Producer", srcport="outport", 
		tgtcomp="Consumer", tgtport="inport",
		connpolicy = { type=1, size=10 } } }

create_conns(d, conn_spec)

rtt.Logger.setlevel("Info")

l.Consumer:start()
l.Producer:start()

