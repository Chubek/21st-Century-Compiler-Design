-- Chapter 76: Lua 5.4 extension and embedded-host boundary checks.
local b = require('bookffi')
assert(b.add(19, 23) == 42)
assert(b.add(-2147483648, 0) == -2147483648)
assert(not pcall(b.add, 2147483647, 1))
assert(not pcall(b.add, 1.0, 2))
assert(not pcall(b.add, '1', 2))
local x = b.create('A\0\255B')
collectgarbage('collect')
assert(x:bytes() == 'A\0\255B')
x:close(); x:close()
assert(not pcall(x.bytes, x))
assert(b.create(''):bytes() == '')
assert(not pcall(b.create, string.rep('x', 1048577)))
assert(b.visit(5, function(i) return b.add(i, 2) end) == 20)
assert(b.visit(0, function() error('unreachable') end) == 0)
assert(b.visit(3, function(i)
  return b.visit(i, function(j) return j+1 end)
end) == 4)
assert(not pcall(b.visit, 1, function() return 1.0 end))
assert(not pcall(b.visit, 1, function() return 2147483648 end))
assert(not pcall(b.visit, -1, function(i) return i end))
local marker, seen = {}, {}
local ok, err = pcall(b.visit, 5, function(i)
  seen[#seen+1] = i
  if i == 2 then error(marker) end
  collectgarbage('collect')
  return i
end)
assert(not ok and err == marker and #seen == 3 and seen[3] == 2)
local saved
do
  local value <close> = b.create('scope')
  saved = value
  assert(value:bytes() == 'scope')
end
assert(not pcall(saved.bytes, saved))
for i = 1, 1000 do
  local value = b.create(string.char(i % 256))
  if i % 2 == 0 then value:close() end
end
collectgarbage('collect')
print('Lua values, bytes, lifetime, reentrancy, errors, and close scopes passed')
