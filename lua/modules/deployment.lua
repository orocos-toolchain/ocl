

require("lfs")

local io = io
local lfs = lfs
local string = string
local assert, print = assert, print

module("deployment")


--
-- recursively find rtt components
--

local rtt_comp_regexp = "liborocos-[%a%d%-_]+.so.%d+.%d+.%d+"

function find_comps(dir)
   local res = {}

   local function filetype(file)
      local attr = lfs.attributes(file)
      return attr.mode
   end

   local function __find_comps_rec(dir)
      for file in lfs.dir(dir) do
	 local ftype = filetype(dir .. "/" .. file)
	 
	 if ftype == 'file' and string.find(file, rtt_comp_regexp) then
	    res[#res+1] = dir .. "/" .. file
	 elseif file ~= "." and file ~= ".." and ftype == 'directory' then
	    __find_comps_rec(dir .. "/" .. file)
	 end
      end
   end

   assert(filetype(dir) == 'directory', "find_comps: needs a directory")
   __find_comps_rec(dir)
   return res
end

function connectTwoPorts(tc1, p1str, tc2, p2str, conn_policy)
   p1 = tc1:getPort(p1str)
   p2 = tc2:getPort(p2str)
   return p1:connectTo(p2, conn_policy)
end