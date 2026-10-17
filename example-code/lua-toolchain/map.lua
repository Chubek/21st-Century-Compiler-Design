-- Source chapter 60: ordered rules, region capacities and permissions.
local page = 4096
return {
  entry = "_start",
  regions = {
    {name="code", origin=0x401000, length=4*page, flags="rx"},
    {name="data", origin=0x406000, length=4*page, flags="rw"},
  },
  sections = {
    {name=".text", region="code", align=16},
    {name=".rodata", region="code", align=16},
    {name=".data", region="data", align=8},
    {name=".bss", region="data", align=16},
  },
}
