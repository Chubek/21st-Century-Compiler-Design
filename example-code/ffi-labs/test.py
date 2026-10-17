"""Build and execute the FFI case studies; Python 3.11+, GCC/Clang, libffi, Lua 5.4.
Run from any directory. Outputs are temporary under /tmp/opencode. Linux required.
"""
from pathlib import Path
import os
import shlex
import subprocess
import sys
import sysconfig
import tempfile
import unittest

HERE = Path(__file__).resolve().parent

def run(args, **kwargs):
    p = subprocess.run([str(a) for a in args], check=False, text=True,
                       capture_output=True, timeout=90, **kwargs)
    if p.returncode:
        raise AssertionError(f'{args}\nexit {p.returncode}\n{p.stdout}\n{p.stderr}')
    if 'runtime error:' in p.stderr or 'ERROR: AddressSanitizer' in p.stderr:
        raise AssertionError(p.stderr)
    return p

def pkg(name, option):
    return shlex.split(run(['pkg-config', option, name]).stdout)

class Boundaries(unittest.TestCase):
    def test_native_and_language_matrix(self):
        for cc in ('gcc', 'clang'):
            for sanitized in (False, True):
                with self.subTest(cc=cc, sanitized=sanitized), tempfile.TemporaryDirectory(
                        prefix='book-ffi-', dir='/tmp/opencode') as directory:
                    out = Path(directory)
                    flags = [cc, '-std=c11', '-O2', '-g', '-Wall', '-Wextra', '-Wpedantic',
                             '-Werror', '-fPIC'] + pkg('libffi', '--cflags')
                    if sanitized:
                        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
                    core = HERE/'bookffi.c'
                    ffi = pkg('libffi', '--libs')
                    env = dict(os.environ, ASAN_OPTIONS='detect_leaks=1')
                    run(flags + ['-shared', core, '-o', out/'libbookffi.so'] + ffi)
                    run(flags + [core, HERE/'native_test.c', '-o', out/'native', '-pthread', '-ldl'] + ffi)
                    run([out/'native', out/'libbookffi.so'], env=env)
                    lua_flags = pkg('lua5.4', '--cflags')
                    lua_libs = pkg('lua5.4', '--libs')
                    run(flags + lua_flags + [core, HERE/'lua_module.c', HERE/'lua_host.c',
                                            '-o', out/'lua-host'] + ffi + lua_libs)
                    run([out/'lua-host', HERE/'check_lua.lua'], env=env)
                    # Extension loading into system interpreters is tested normally;
                    # sanitizer initialization is instead owned by the native host above.
                    if not sanitized:
                        lua_dir=out/'lua'; lua_dir.mkdir()
                        run(flags + lua_flags + ['-shared', core, HERE/'lua_module.c',
                                                '-o', lua_dir/'bookffi.so'] + ffi + lua_libs)
                        run(['lua5.4', HERE/'check_lua.lua'],
                            env=dict(env, LUA_CPATH=str(lua_dir/'?.so')))
                        run(flags + ['-I'+sysconfig.get_path('include'), '-shared', core,
                                     HERE/'python_module.c', '-o', out/'bookffi.abi3.so'] + ffi)
                        run([sys.executable, HERE/'check_python.py'],
                            env=dict(env, PYTHONPATH=str(out), PYTHONDONTWRITEBYTECODE='1'))

if __name__ == '__main__':
    unittest.main(verbosity=2)
