

local assert, print, type, tlsf = assert, print, type, tlsf

assert(type(tlsf.stats) == 'function', "Not a TLSF enabled Lua")

module ("tlsf_ext")

function info()
   local cur, max, tot = tlsf.stats()
   print(("cur=%d (%d%s), max=%d (%d%s), total=%d"):format(cur, ((cur * 100) / tot), '%', max, (max * 100) / tot, '%', tot))
end



