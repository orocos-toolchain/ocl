require("rttlib")

tc=rtt.getTC();

-- The Lua component starts its life in PreOperational, so configure
-- can be used to set stuff up.
function configureHook()
   inport = rtt.InputPort("string", "inport")    -- global variable!
   tc:addEventPort(inport)
   cnt = 0
   return true
end

-- all hooks are optional!
--function startHook() return true end

function updateHook()
   local fs, data = inport:read()
   rtt.log("data received: " .. tostring(data) .. ", flowstatus: " .. fs)
end

-- Ports and properties are the only elements which are not
-- automatically cleaned up. This means this must be done manually for
-- long living components:
function cleanupHook()
   tc:removePort("inport")
   inport:delete()
   print(inport)
end