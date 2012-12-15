--require("Deployer")
-- require("rtt")
rtt.Logger.getlevel()
require("rttlib")
require("luagc")

rtt.Logger.setlevel("Info")

var = rtt.Variable

--d=deployer.new("myDeployer")
d = TC:getPeer("Deployer")

-- setup pretty printing
mt = getmetatable(rtt.Variable.new("int"))
mt.__tostring=rttlib.var2str

mt = getmetatable(d)
mt.__tostring=rttlib.tc2str


print("getName: ", rtt.TaskContext.getName(d))
print("getState: ", d:getState())
-- rtt.TaskContext.delete(d)

function test_gc()
   luagc.full()
   luagc.full()

   t={}
   local function alloc(num)
      for i=1,num do
	 t[#t+1]=rtt.Variable.new("string", string.rep("+", 1000))
      end
   end

   alloc(10)
   t={}
   luagc.full()
   luagc.full()
end

function create_vars()
   print(" --- bool ---")
   x = var.new("bool", true)
   print("toString", x:toString())
   print(x:getType(), x:tolua())

   print(" --- int ---")
   x = var.new("int", -4711)
   print("toString", x:toString())
   print(x:getType(), x:tolua())

   print(" --- uint ---")
   x = var.new("uint", 4712)
   print("toString", x:toString())
   print(x:getType(), x:tolua())

   print(" --- double ---")
   x = var.new("double", 4711.1415)
   print("toString", x:toString())
   print(x:getType(), x:tolua())

   print(" --- char ---")
   x = var.new("char", "Moon")
   print("toString", x:toString())
   print(x:getType(), x:tolua())

   print(" --- string ---")
   x = var.new("string", "my hello world string")
   print("toString", x:toString())
   print(x:getType(), x:tolua())
end

--
function test_ports()
   ip = rtt.InputPort.new("string", "my-port")
   op = rtt.OutputPort.new("string", "my-out-port")

   print("depl ports before:",  table.concat(d:getPortNames(), ', '))
   d:addPort(ip)
   d:addPort(op)
   print("depl ports after:",  table.concat(d:getPortNames(), ', '))
end

function displayCompTypes()
   d:call("displayComponentTypes")
end


function test_var_introspection()
   types = rtt.Variable.getTypes()
   print("known types: ", utils.tab2str(types))
   for _,v in ipairs(types) do
      if v ~= "void" then
	 x = rtt.Variable.new(v)
	 print("\ttesting " .. v .. ":", rttlib.var2str(x))
      end
   end
end

function test_var_update()
   s1=var.new("string", "hello-cruel-evil-world")
   s2=var.new("string", "hello-wonderful-nice-world")
   print("s1: " .. s1:tolua(), "s2: " .. s2:tolua());
   print("update: ", s1:update(s2));
   print("s1: " .. tostring(s1), "s2: " .. tostring(s2));
end

--test_var_introspection()
-- test_var_update()

function test_operators()
   v1 = var.new("int", 33)
   v2 = var.new("int", 66)
   v3 = var.opBinary("+", v1,v2)
   print(v3)
end

test_operators()

function test_logger()
   l = rtt.Logger
   print("getlevel", l.getlevel())
   print("setlevel to Info", l.setlevel("Info"))
   print("getlevel", l.getlevel())
   print("logging something", l.log("something"))
end


function test_index_mm()
   cp = var.new("ConnPolicy")
   print("ConnPolicy cp ", cp)
   print("cp.data_size", cp.data_size)
end
-- test_index_mm()

function test_compound_data()
   for i,t in ipairs(rtt.Variable.getTypes()) do
      local v = var.new(t)
      print(t, tostring(v))
   end
end

-- test_compound_data()

function test_ports()
   op = rtt.OutputPort.new("string", "outport1")
   ip = rtt.InputPort.new("string", "inport1")

   TC:addPort(op)
   TC:addPort(ip)

   depl = TC:getPeer("Deployer")
   print("connecting ports: ",
	 depl:call("connectTwoPorts",
		   var.new("string", "lua"),
		   var.new("string", "outport1"),
		   var.new("string", "lua"),
		   var.new("string", "inport1")))

   local res = var.new("string")

   for i=1,10 do
      local mes = "data_" .. tostring(i)
      print("writing... " .. mes)
      op:write(var.new("string", mes))
      ip:read(res)
      print("reading... " .. tostring(res))
   end
end

-- test_ports()

function test_props()
   p = rtt.Property.new("string", "myprop", "my special property")
   p:set(var.new("string", "hatschi!"))
   print(p)
   TC:addProperty(p)
end

test_props()
