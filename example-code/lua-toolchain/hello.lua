-- Lua generates exact byte data; assembler encodes RIP-relative addressing.
local message = "Lua maps, ELF runs.\n"
emit(string.format([[
.global _start
.text
_start:
mov eax, 1
mov edi, 1
lea rsi, message
mov edx, %d
syscall
mov eax, 60
xor edi, edi
syscall
.rodata
message:
]], #message))
for i = 1, #message do
  emit(string.format(".byte %d", message:byte(i)))
end
