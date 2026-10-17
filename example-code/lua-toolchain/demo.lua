-- Chapter 56: Lua expands a reusable macro and emits data bytes.
local serial = 0
local function countdown(register, count)
  serial = serial + 1
  local label = string.format(".loop_%d", serial)
  emit(string.format("mov %s, %d", register, count))
  emit(label .. ":")
  emit(string.format("sub %s, 1", register))
  emit("jnz " .. label)
end
emit([[
.text
.global _start
.extern answer
_start:
]])
countdown("ecx", 3)
emit([[
call answer
mov edi, eax
mov eax, 60
syscall
.data
.global answer_pointer
answer_pointer:
.quad answer
.bss
.align 16
scratch:
.zero 32
]])
