require("rttlib")

tc=rtt.getTC();

function configureHook()
   outport = rtt.OutputPort.new("string", "outport")    -- global variable!
   tc:addPort(outport)
   cnt = 0
   return true
end

function startHook() return true end

function updateHook()
   outport:write("message number " .. cnt)
   cnt = cnt + 1
end

function cleanupHook()
   tc:removePort("outport")
   outport:delete()
end