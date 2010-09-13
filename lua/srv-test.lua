require("rttlib")

rttlib.color=true
var = rtt.Variable
d=TC:getPeer("deployer")

print(d:call("loadService", "lua", "scripting"))

function ppsrv(s)
   print("--- Service: " .. s:getName() .. " ---")
   print("SubServices: ", table.concat(s:getProviderNames(), ", "))
   print("Operations:  ", table.concat(s:getOperationNames(), ", "))
   print("Ports:       ", table.concat(s:getProviderNames(), ", "))
end

ppsrv(TC:provides())

s=TC:provides():provides("scripting")

ppsrv(s)
op=s:getOperation("hasProgram")