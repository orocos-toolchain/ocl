require("rttlib")

tc=rtt.getTC();

function configureHook()
   inport = rtt.InputPort.new("string", "inport")    -- global variable!
   tc:addEventPort(inport)
   cnt = 0
end

function startHook()
end

function updateHook()
   local fs, data = inport:read()
   rtt.Logger.log("data received: " .. tostring(data) .. ", flowstatus: " .. fs)
end

function stopHook()
end

function cleanupHook()
   -- remove port
   -- destruct port
end