-- Some code to make life with ROS easier.

module("rttros", package.seeall)

-- Copied from roslua: http://www.ros.org/wiki/roslua

--- Get path for a package.
-- Uses rospack to find the path to a certain package. The path is cached so
-- that consecutive calls will not trigger another rospack execution, but are
-- rather handled directly from the cache. An error is thrown if the package
-- cannot be found.
-- @return path to give package

local rospack_path_cache = {}

function find_rospack_roslua(package)
   if not rospack_path_cache[package] then
      local p = io.popen("rospack find " .. package .. " 2>/dev/null")
      local path = p:read("*a")
      -- strip trailing newline
      rospack_path_cache[package] = string.gsub(path, "^(.+)\n$", "%1")
      p:close()
   end

   assert(rospack_path_cache[package], "Package path could not be found for " .. package)
   assert(rospack_path_cache[package] ~= "", "Package path could not be found for " .. package)
   return rospack_path_cache[package]
end

local rtt_rospack_find=false
function find_rospack(package)
   if not rtt_rospack_find then
      if not (rtt and rttlib) then
	 error("find_rospack: not an rttlua _or_ rttlib not loaded.")
      end
      depl = rttlib.findpeer("deployer") or rttlib.findpeer("Deployer")
      if not depl then error("find_rospack: failed to find a deployer") end
      depl:import("rtt_rospack")
      rtt_rospack_find=rtt.provides("rospack"):getOperation("find")
   end
   local res = rtt_rospack_find(package)
   if res~="" then return res else return false end
end

-- Help Markus' poor, confused brain:
rospack_find=find_rospack
