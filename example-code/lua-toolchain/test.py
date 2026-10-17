"""C11/Lua tools: native, ELF, differential, malformed-input, and range checks.
Run with Python 3 on x86-64 Linux; GCC, Clang, binutils, pkg-config, Lua 5.4.
All artifacts are confined to a temporary directory under /tmp/opencode.
"""
from pathlib import Path
import os
import struct
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent


def run(args, expected=0):
    p = subprocess.run(list(map(str, args)), capture_output=True, timeout=30,
                       env={**os.environ, 'ASAN_OPTIONS': 'detect_leaks=1'})
    if p.returncode != expected:
        raise AssertionError((args, p.returncode, p.stdout.decode(errors='replace'), p.stderr.decode(errors='replace')))
    if b'Sanitizer' in p.stderr or b'runtime error:' in p.stderr:
        raise AssertionError(p.stderr)
    return p


def sections(data):
    start = struct.unpack_from('<Q', data, 40)[0]
    n, names = struct.unpack_from('<HH', data, 60)
    headers = [struct.unpack_from('<IIQQQQIIQQ', data, start + 64*i) for i in range(n)]
    table = headers[names]
    strings = data[table[4]:table[4]+table[5]]
    return {strings[h[0]:].split(b'\0')[0].decode(): h for h in headers}


def loads(data):
    start = struct.unpack_from('<Q', data, 32)[0]
    n = struct.unpack_from('<H', data, 56)[0]
    return [h for i in range(n) if (h := struct.unpack_from('<IIQQQQQQ', data, start + 56*i))[0] == 1]


class Tools(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix='lua-toolchain-', dir='/tmp/opencode')
        cls.root = Path(cls.tmp.name)
        cls.configs = []
        for cc in ('gcc', 'clang'):
            for san in (False, True):
                out = cls.root / (cc + ('-san' if san else ''))
                env = {**os.environ, 'CFLAGS': '-fsanitize=address,undefined -fno-omit-frame-pointer -g' if san else '-O2'}
                subprocess.run(['sh', str(HERE/'build.sh'), str(out), cc], env=env, check=True, capture_output=True)
                cls.configs.append(out)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def assemble(self, tools, name, text):
        s = tools/(name+'.asm'); s.write_text(text)
        o = tools/(name+'.o'); run([tools/'luaas', '--asm', s, o]); return o

    def link(self, tools, inputs, maptext=None, expected=0):
        m = tools/'layout.lua'; m.write_text(maptext or (HERE/'map.lua').read_text())
        image = tools/'image'; image.unlink(missing_ok=True)
        p = run([tools/'luald', m, image, *inputs], expected)
        if expected:
            self.assertFalse(image.exists())
        return image, p

    def test_demo_and_independent_relocation_math(self):
        hashes = []
        for tools in self.configs:
            a, b = tools/'demo.o', tools/'answer.o'
            run([tools/'luaas', HERE/'demo.lua', a]); run([tools/'luaas', HERE/'answer.lua', b])
            image, p = self.link(tools, [a, b]); run([image], 42)
            data = image.read_bytes(); hashes.append(data)
            code, writable = loads(data)
            self.assertEqual((code[3], code[5], code[6]), (0x401000, 0x26, 0x26))
            self.assertEqual((writable[3], writable[5], writable[6]), (0x406000, 8, 48))
            # jnz field at 13 targets 5; call field at 18 targets 32.
            self.assertEqual(struct.unpack_from('<i', data, code[2]+13)[0], -12)
            self.assertEqual(struct.unpack_from('<i', data, code[2]+18)[0], 10)
            self.assertEqual(struct.unpack_from('<Q', data, writable[2])[0], 0x401020)
            run(['readelf', '-h', '-S', '-s', '-r', a]); run(['readelf', '-h', '-l', image])
            # Our ET_REL is accepted by a production linker.
            native = tools/'gnu-linked'; run(['ld', '-o', native, '-e', '_start', a, b]); run([native], 42)
        self.assertTrue(all(x == hashes[0] for x in hashes))

    def test_encoding_against_gnu_as(self):
        names = ['rax','rcx','rdx','rbx','rsp','rbp','rsi','rdi']
        small = ['eax','ecx','edx','ebx','esp','ebp','esi','edi']
        ours, gnu = [], ['.intel_syntax noprefix', '.text']
        for group in (names, small):
            for a in group:
                for b in group:
                    for op in ('mov', 'xor'):
                        ours.append(f'{op} {a}, {b}'); gnu.append(ours[-1])
        for a in names:
            ours.append(f'mov {a}, 0x123456789'); gnu.append(f'movabs {a}, 0x123456789')
        for a in small:
            ours.append(f'mov {a}, 0x12345678'); gnu.append(ours[-1])
        gnu += ['ret', 'syscall', 'nop', 'ud2']; ours += ['ret', 'syscall', 'nop', 'ud2']
        for tools in self.configs:
            o = self.assemble(tools, 'enc', '\n'.join(ours))
            s = tools/'reference.s'; s.write_text('\n'.join(gnu)+'\n')
            ref = tools/'reference.o'; run(['as', '--64', s, '-o', ref])
            def code(path):
                data = path.read_bytes(); h = sections(data)['.text']; return data[h[4]:h[4]+h[5]]
            self.assertEqual(code(o), code(ref))

    def test_lua_byte_generation_and_rebased_layout(self):
        for tools in self.configs:
            o=tools/'hello.o'; run([tools/'luaas',HERE/'hello.lua',o])
            for base in ('0x401000','0x601000'):
                m=(HERE/'map.lua').read_text().replace('0x401000',base)
                image,_=self.link(tools,[o],m)
                self.assertEqual(run([image]).stdout,b'Lua maps, ELF runs.\n')

    def test_gnu_objects_and_zero_fill(self):
        # GNU emits PLT32 for the call and PC32 for RIP-relative BSS access.
        asm = (HERE/'zero-check.s').read_text()
        for tools in self.configs:
            s = tools/'foreign.s'; s.write_text(asm); o = tools/'foreign.o'
            run(['as', '--64', '-mx86-used-note=no', s, '-o', o])
            helper = self.assemble(tools, 'helper', '.global helper\nhelper: mov eax, 37\nret\n')
            image, _ = self.link(tools, [o, helper]); run([image], 37)

    def test_local_and_weak_resolution(self):
        for tools in self.configs:
            main = self.assemble(tools, 'main', '.global _start\n.extern f\n_start: call f\nmov edi, eax\nmov eax, 60\nsyscall\nlocal: ret\n.data\n.weak absent\n.quad absent\n')
            weak = self.assemble(tools, 'weak', '.weak f\nf: mov eax, 7\nret\nlocal: ret\n')
            strong = self.assemble(tools, 'strong', '.global f\nf: mov eax, 19\nret\nlocal: ret\n')
            for inputs, result in (([main,weak],7), ([main,weak,strong],19), ([main,strong,weak],19)):
                image,_ = self.link(tools, inputs); run([image],result)
            self.link(tools,[main],expected=2)
            self.link(tools,[main,strong,strong],expected=2)

    def test_map_constraints_and_reordering(self):
        normal = (HERE/'map.lua').read_text()
        for tools in self.configs:
            o = self.assemble(tools,'maptest','.global _start\n_start: mov eax,60\nxor edi,edi\nsyscall\n.data\n.quad 12\n.bss\n.zero 16\n')
            for bad in (normal.replace('0x406000','0x401000'),
                        normal.replace('0x401000','0x401001'),
                        normal.replace('length=4*page','length=0'),
                        normal.replace('align=16','align=3'),
                        normal.replace('flags="rw"','flags="r"'),
                        normal.replace('name=".text"','name=".oops"'),
                        normal.replace('entry = "_start"','entry = "missing"'),
                        'return setmetatable({}, {__index=function() error("oops") end})'):
                self.link(tools,[o],bad,2)
            # Reversed address order is sorted for PT_LOAD output, not placement.
            alternate = normal.replace('0x401000','0x501000').replace('0x406000','0x301000')
            image,_ = self.link(tools,[o],alternate); run([image],0)
            ph = loads(image.read_bytes()); self.assertLess(ph[0][3],ph[1][3])

    def test_relocation_boundaries(self):
        for tools in self.configs:
            o = self.assemble(tools,'ranges','.global _start\n.extern target\n_start: call target\nret\n.data\n.quad target\n.long target\n')
            other = self.assemble(tools,'target','.data\n.global target\ntarget: .quad 0\n')
            # Far PC32 overflows; full-width absolute relocation alone would fit.
            far = (HERE/'map.lua').read_text().replace('0x406000','0x90000000')
            _,p = self.link(tools,[o,other],far,2); self.assertIn(b'relocation overflow',p.stderr)
            absref = self.assemble(tools,'absref','.global _start\n.extern target\n_start: ret\n.data\n.long target\n')
            _,p = self.link(tools,[absref,other],far.replace('0x90000000','0x100000000'),2)
            self.assertIn(b'relocation overflow',p.stderr)
            edge=self.assemble(tools,'edge','.global _start\n_start: call target\ntarget: ret\n')
            original=edge.read_bytes(); at=sections(original)['.rela.text'][4]+16
            for value,valid in ((-2**31,True),(2**31-1,True),(-2**31-1,False),(2**31,False)):
                data=bytearray(original); struct.pack_into('<q',data,at,value-4); edge.write_bytes(data)
                image,_=self.link(tools,[edge],expected=0 if valid else 2)
                if valid:
                    binary=image.read_bytes(); segment=loads(binary)[0]
                    self.assertEqual(struct.unpack_from('<i',binary,segment[2]+1)[0],value)

    def test_assembler_rejections(self):
        bad = ['label: nop\nlabel: ret', 'jmp missing', '.byte 256', '.long 4294967296',
               '.bss\n.byte 1', '.align 3', '.zero -1', 'mov rax, eax', 'mov r8, 1',
               'add eax, 2147483648', '.global x\n.weak x', 'ret junk', 'unknown x',
               '.quad x-9223372036854775808', 'call 4', '.zero 16777217']
        for tools in self.configs:
            for source in bad:
                s = tools/'bad.asm'; s.write_text(source); out = tools/'bad.o'; out.unlink(missing_ok=True)
                run([tools/'luaas','--asm',s,out],2); self.assertFalse(out.exists())
            for lua in ('emit("nop")\nerror("stop")', 'emit("x\\0y")', 'error({})'):
                s = tools/'bad.lua'; s.write_text(lua)
                run([tools/'luaas',s,tools/'bad.o'],2); self.assertFalse((tools/'bad.o').exists())

    def test_malformed_elf(self):
        for tools in self.configs:
            valid = self.assemble(tools,'valid','.global _start\n_start: ret\n.data\n.quad _start\n').read_bytes()
            variants = [valid[:i] for i in (0,4,16,63,len(valid)-1)]
            for off,width,value in ((4,1,1),(5,1,2),(18,2,3),(40,8,2**64-1),(60,2,65535)):
                b=bytearray(valid); b[off:off+width]=value.to_bytes(width,'little'); variants.append(b)
            headers=sections(valid)
            rela=headers['.rela.data'][4]
            for off,width,value in ((rela,8,8),(rela+8,8,(5000<<32)|1),(rela+8,4,99)):
                b=bytearray(valid); b[off:off+width]=value.to_bytes(width,'little'); variants.append(b)
            for b in variants:
                p=tools/'broken.o'; p.write_bytes(b); self.link(tools,[p],expected=2)

    def test_native_dynamic_linking_and_tls(self):
        for cc in ('gcc','clang'):
            shared=self.root/(cc+'-tls.so'); exe=self.root/(cc+'-loader')
            flags=['-std=c11','-Wall','-Wextra','-Wpedantic','-Werror']
            run([cc,*flags,'-fPIC','-shared',HERE/'dynamic-lib.c','-o',shared])
            run([cc,*flags,HERE/'dynamic-main.c','-ldl','-pthread','-o',exe])
            self.assertIn(b'independent TLS counters',run([exe,shared]).stdout)
            self.assertIn(b'TLS',run(['readelf','-l',shared]).stdout)


if __name__ == '__main__':
    unittest.main(verbosity=2)
