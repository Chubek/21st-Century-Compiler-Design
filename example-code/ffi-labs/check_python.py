"""Chapter 76: runtime-boundary checks, invoked with the built module on PYTHONPATH."""
import gc
import importlib
import sys
import bookffi as b

def raises(kind, function, *args):
    try:
        function(*args)
    except kind:
        return
    raise AssertionError((kind, function, args))

assert b.calls() == 0
for x in (-2**31, -1, 0, 1, 2**31-1):
    for y in (-2**31, -1, 0, 1, 2**31-1):
        if -2**31 <= x+y < 2**31:
            assert b.add(x, y) == x+y
        else:
            raises(OverflowError, b.add, x, y)
for value in (True, 1.0, "1", None):
    raises(TypeError, b.add, value, 0)
raises(OverflowError, b.add, 2**80, 0)
data = bytearray(b"A\x00\xffB")
owner = b.create(data)
data[0] = 90
data.extend(b"!")  # Export released: resizing is permitted after create returns.
assert b.bytes(owner) == b"A\x00\xffB"
gc.collect()
assert b.bytes(owner) == b"A\x00\xffB"
b.close(owner); b.close(owner)
raises(ValueError, b.bytes, owner)
raises(ValueError, b.bytes, object())
assert b.bytes(b.create(b"")) == b""
raises(BufferError, b.create, memoryview(b"abcd")[::2])
raises(OverflowError, b.create, b"x" * (1048576+1))
assert b.visit(5, lambda i: b.add(i, 2)) == 20
assert b.visit(0, lambda i: 1/0) == 0
assert b.visit(3, lambda i: b.visit(i, lambda j: j+1)) == 4
for bad in (lambda i: True, lambda i: 1.5):
    raises(TypeError, b.visit, 1, bad)
raises(OverflowError, b.visit, 1, lambda i: 2**40)
raises(OverflowError, b.visit, -1, lambda i: i)
raises(TypeError, b.visit, 1, None)
error = RuntimeError("callback marker")
seen = []
def failing(i):
    seen.append(i)
    if i == 2:
        raise error
    gc.collect()
    return i
try:
    b.visit(5, failing)
except RuntimeError as caught:
    assert caught is error
else:
    raise AssertionError("missing callback failure")
assert seen == [0, 1, 2]
old_count = b.calls()
del sys.modules['bookffi']
other = importlib.import_module('bookffi')
assert other is not b and other.calls() == 0
assert other.add(4,5) == 9 and other.calls() == 1 and b.calls() == old_count
for i in range(1000):
    x = b.create(bytes([i % 256]))
    if i % 2:
        b.close(x)
del x, owner
gc.collect()
print("Python values, bytes, lifetime, reentrancy, errors, and module isolation passed")
