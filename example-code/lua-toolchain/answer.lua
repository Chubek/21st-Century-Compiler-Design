-- A separate object supplies the definition requested by demo.lua.
emit([[
.text
.global answer
answer:
mov eax, 42
ret
]])
