require("rttlib")

tc=rtt.getTC();

function configureHook()
   outport = rtt.OutputPort.new("string", "outport")    -- global variable!
   tc:addPort(outport)
   cnt = 0
end

function startHook()
end

function updateHook()
   outport:write("message number " .. cnt)
   cnt = cnt + 1
end

function stopHook()
end

function cleanupHook()
   tc:removePort("outport")
   outport:delete()
end