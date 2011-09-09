--- Some TLSF related helpers

local assert, print, type, tlsf = assert, print, type, tlsf

assert(type(tlsf) == 'table', "Not a TLSF enabled Lua")

module ("tlsf_ext")

--- Pretty print memory status.
function info()
   local cur, max, tot = tlsf.stats()
   print(("tlsf stats: cur=%d (%d%s), max=%d (%d%s), total=%d"):format(cur, ((cur * 100) / tot), '%', max, (max * 100) / tot, '%', tot))
end



