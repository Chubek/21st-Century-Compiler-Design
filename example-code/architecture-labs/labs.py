"""Executable models for source chapters 61--68; Python 3.10+, standard library.

These are small reference implementations, not an integrated production JIT.
Integer rewrite semantics are total arithmetic modulo 256. See README.md.
"""
from dataclasses import dataclass, field
from collections import defaultdict
from itertools import permutations


# Chapter 61: tracked, demand-driven queries with exact result comparison.
@dataclass
class Cached:
    value: object
    dependencies: tuple
    changed: int
    verified: int


class Queries:
    def __init__(self, inputs, providers):
        self.inputs = dict(inputs)
        self.providers = dict(providers)
        self.revision = 0
        self.input_changed = dict.fromkeys(inputs, 0)
        self.cache = {}
        self.active = []
        self.reads = []
        self.calls = defaultdict(int)

    def edit(self, key, value):
        if self.active:
            raise RuntimeError('edit during query')
        if key not in self.inputs:
            raise KeyError(key)
        self.revision += 1
        if self.inputs[key] != value:
            self.inputs[key] = value
            self.input_changed[key] = self.revision

    def changed(self, key):
        return self.input_changed[key] if key in self.inputs else self.cache[key].changed

    def get(self, key):
        if self.reads:
            self.reads[-1].append(key)
        if key in self.inputs:
            return self.inputs[key]
        if key in self.active:
            raise RuntimeError('cyclic query')
        old = self.cache.get(key)
        if old and old.verified == self.revision:
            return old.value
        self.active.append(key)
        self.reads.append([])
        try:
            if old:
                valid = True
                for dep in old.dependencies:
                    self.get(dep)
                    if self.changed(dep) > old.verified:
                        valid = False
                        break  # Remaining old reads may no longer be legal.
                if valid:
                    old.verified = self.revision
                    return old.value
            self.reads[-1] = []
            self.calls[key] += 1
            value = self.providers[key](self.get)
            dependencies = tuple(dict.fromkeys(self.reads[-1]))
            stamp = old.changed if old and old.value == value else self.revision
            self.cache[key] = Cached(value, dependencies, stamp, self.revision)
            return value
        finally:
            self.reads.pop()
            self.active.pop()


# Chapter 62: a small actual e-graph with congruence rebuilding and extraction.
class EGraph:
    def __init__(self):
        self.parent = []
        self.nodes = []  # (owner, (opcode, payload, child-class IDs))
        self.memo = {}

    def find(self, x):
        while self.parent[x] != x:
            self.parent[x] = self.parent[self.parent[x]]
            x = self.parent[x]
        return x

    def union(self, a, b):
        a, b = self.find(a), self.find(b)
        if a == b:
            return False
        self.parent[max(a, b)] = min(a, b)
        return True

    def canonical(self, node):
        op, payload, children = node
        return op, payload, tuple(self.find(c) for c in children)

    def add(self, op, children=(), payload=None):
        node = self.canonical((op, payload, children))
        if node in self.memo:
            return self.find(self.memo[node])
        owner = len(self.parent)
        self.parent.append(owner)
        self.nodes.append((owner, node))
        self.memo[node] = owner
        return owner

    def rebuild(self):
        while True:
            memo, changed = {}, False
            for owner, node in self.nodes:
                node = self.canonical(node)
                if node in memo:
                    changed |= self.union(owner, memo[node])
                else:
                    memo[node] = owner
            if not changed:
                self.memo = {n: self.find(c) for n, c in memo.items()}
                return

    def saturate(self, rounds=20, node_limit=1000):
        for _ in range(rounds):
            self.rebuild()
            constants = {}
            for owner, (op, payload, _) in self.nodes:
                if op == 'const':
                    root = self.find(owner)
                    if root in constants and constants[root] != payload:
                        raise ValueError('inconsistent equality')
                    constants[root] = payload
            changed = False
            for owner, raw in list(self.nodes):
                op, _, children = self.canonical(raw)
                if op not in ('add', 'mul'):
                    continue
                a, b = children
                ca, cb = constants.get(a), constants.get(b)
                if ca is not None and cb is not None:
                    v = (ca + cb if op == 'add' else ca * cb) & 255
                    changed |= self.union(owner, self.add('const', payload=v))
                identity = 0 if op == 'add' else 1
                if ca == identity:
                    changed |= self.union(owner, b)
                if cb == identity:
                    changed |= self.union(owner, a)
                if op == 'mul' and (ca == 0 or cb == 0):
                    changed |= self.union(owner, self.add('const', payload=0))
                if len(self.nodes) > node_limit:
                    self.rebuild()
                    return False
            if not changed:
                self.rebuild()
                return True
        self.rebuild()
        return False

    def extract(self, root):
        # Positive unit node costs: cycles cannot improve a finite witness.
        best = {}
        for _ in range(len(self.parent) + 1):
            changed = False
            for owner, raw in self.nodes:
                op, payload, children = self.canonical(raw)
                if not all(c in best for c in children):
                    continue
                cost = 1 + sum(best[c][0] for c in children)
                term = (op, payload, tuple(best[c][1] for c in children))
                owner = self.find(owner)
                if owner not in best or cost < best[owner][0]:
                    best[owner] = cost, term
                    changed = True
            if not changed:
                break
        if self.find(root) not in best:
            raise ValueError('no finite extraction')
        return best[self.find(root)]


def eval_term(t, x):
    op, payload, children = t
    if op == 'var':
        return x
    if op == 'const':
        return payload
    a, b = (eval_term(c, x) for c in children)
    return (a + b if op == 'add' else a * b) & 255


# Chapter 63: concrete observable trace and checked effect reordering.
@dataclass(frozen=True)
class Effect:
    reads: frozenset = frozenset()
    writes: frozenset = frozenset()
    observable: bool = False
    may_trap: bool = False
    may_diverge: bool = False


def commute(a, b):
    return (not (a.observable or b.observable or a.may_trap or b.may_trap
                 or a.may_diverge or b.may_diverge)
            and not (a.writes & (b.reads | b.writes))
            and not (b.writes & a.reads))


def memory_program(p, q, reverse=False):
    memory = {p: 0, q: 0}
    ops = [(p, 9), (q, 4)]
    if reverse:
        ops.reverse()
    for address, value in ops:
        memory[address] = value
    return memory[p], tuple(sorted(memory.items()))


# Chapter 64: lift a precise real x86 subset into single-assignment values.
def lift(code):
    pc, value, rows = 0, 'input', []
    while pc < len(code):
        op = code[pc]
        if op not in (0x04, 0x34, 0xB0) or pc + 1 >= len(code):
            raise ValueError('unsupported or truncated instruction')
        dest = f'v{len(rows)+1}'
        rows.append((dest, {0x04: 'add8', 0x34: 'xor8', 0xB0: 'mov8'}[op],
                     value, code[pc+1], pc))
        value = dest
        pc += 2
    return rows


def add8(a, b):
    r = (a+b) & 255
    return r, {'CF': int(a+b > 255), 'PF': int(r.bit_count() % 2 == 0),
               'AF': ((a ^ b ^ r) >> 4) & 1, 'ZF': int(r == 0),
               'SF': r >> 7, 'OF': ((~(a ^ b) & (a ^ r)) >> 7) & 1}


def execute_lifted(rows, rax, initial_flags):
    values = {'input': rax & 255}
    flags = dict(initial_flags)
    last = 'input'
    for dest, op, source, imm, _ in rows:
        a = values[source]
        if op == 'add8':
            result, flags = add8(a, imm)
        elif op == 'xor8':
            result = a ^ imm
            flags = dict(CF=0, OF=0, ZF=int(result == 0), SF=result >> 7,
                         PF=int(result.bit_count() % 2 == 0), AF=None)
        else:
            result = imm
        values[dest] = result
        last = dest
    return (rax & ~255) | values[last], flags


# Chapter 65: quickening and residual bytecode from expression semantics.
class AddSite:
    def __init__(self):
        self.state = 'cold'
        self.misses = 0

    def __call__(self, a, b):
        ints = type(a) is int and type(b) is int
        if self.state == 'cold':
            self.state = 'int' if ints else 'generic'
        if self.state == 'int' and not ints:
            self.misses += 1
            self.state = 'generic'
        if ints:
            return a + b
        if type(a) is str and type(b) is str:
            return a + b
        raise TypeError('addition requires two integers or two strings')


def interpret(expr, x):
    if expr[0] == 'const':
        return expr[1]
    if expr[0] == 'input':
        return x
    a, b = interpret(expr[1], x), interpret(expr[2], x)
    return a+b if expr[0] == 'add' else a*b


def specialize(expr):
    # Static syntax, dynamic input. Returns residual stack bytecode.
    if expr[0] in ('const', 'input'):
        return [expr]
    return specialize(expr[1]) + specialize(expr[2]) + [(expr[0],)]


def run_bytecode(code, x):
    stack = []
    for ins in code:
        if ins[0] == 'input':
            stack.append(x)
        elif ins[0] == 'const':
            stack.append(ins[1])
        else:
            b, a = stack.pop(), stack.pop()
            stack.append(a+b if ins[0] == 'add' else a*b)
    if len(stack) != 1:
        raise ValueError('invalid final stack')
    return stack[0]


# Chapter 66: OSR and deoptimization at one precise loop-header continuation.
@dataclass
class Frame:
    i: int = 0
    total: int = 0
    trace: list = field(default_factory=list)


def baseline(items, frame=None):
    f = frame or Frame()
    while f.i < len(items):
        value = items[f.i]
        if type(value) is not int:
            return ('type-error', f.i, f.total, tuple(f.trace))
        f.total += value
        f.trace.append((f.i, f.total))
        f.i += 1
    return ('ok', f.i, f.total, tuple(f.trace))


def optimized(items, entry=0):
    f = Frame()
    # Execute a baseline prefix, then perform OSR at the loop header.
    while f.i < min(entry, len(items)):
        if type(items[f.i]) is not int:
            return baseline(items, f)
        f.total += items[f.i]
        f.trace.append((f.i, f.total))
        f.i += 1
    while f.i < len(items):
        value = items[f.i]
        # Narrow fast accumulator; failed addition has not committed effects.
        if type(value) is not int or not -128 <= f.total + value <= 127:
            recovered = Frame(f.i, f.total, list(f.trace))
            return baseline(items, recovered)
        f.total += value
        f.trace.append((f.i, f.total))
        f.i += 1
    return ('ok', f.i, f.total, tuple(f.trace))


def materialize(recipes):
    # Two passes preserve shared identity and cycles among virtual objects.
    objects = {name: {} for name in recipes}
    for name, fields in recipes.items():
        for key, (kind, value) in fields.items():
            objects[name][key] = objects[value] if kind == 'ref' else value
    return objects


# Chapter 67: serialized-owner publication, leases, and representation migration.
@dataclass
class Version:
    epoch: int
    body: str
    active: int = 0
    retired: bool = False


class Runtime:
    def __init__(self):
        self.epoch = 0
        self.current = Version(0, 'generic')
        self.versions = [self.current]

    def acquire(self):
        self.current.active += 1
        return self.current

    def release(self, version):
        if version.active <= 0:
            raise ValueError('unbalanced lease')
        version.active -= 1

    def edit(self):
        self.epoch += 1
        self.publish(self.epoch, 'generic')

    def publish(self, request_epoch, body):
        if request_epoch != self.epoch:
            return False
        self.current.retired = True
        self.current = Version(self.epoch, body)
        self.versions.append(self.current)
        return True

    def collect(self):
        self.versions = [v for v in self.versions if not v.retired or v.active]


class ArrayHandle:
    def __init__(self, values):
        self.length = len(values)
        self.storage = list(values)
        self.sparse = False

    def migrate(self, sparse):
        values = [self.get(i) for i in range(self.length)]
        replacement = {i: v for i, v in enumerate(values) if v != 0} if sparse else values
        self.storage, self.sparse = replacement, sparse

    def get(self, i):
        if not 0 <= i < self.length:
            raise IndexError(i)
        return self.storage.get(i, 0) if self.sparse else self.storage[i]


# Chapter 68: exact toy placement and a fitted cost model on synthetic data.
def placement(weights):
    best = None
    for order in permutations(range(len(weights))):
        at = {node: i for i, node in enumerate(order)}
        cost = sum(weights[a][b] * abs(at[a]-at[b])
                   for a in range(len(weights)) for b in range(a+1, len(weights)))
        if best is None or (cost, order) < best:
            best = cost, order
    return best


def fit_line(samples):
    n = len(samples)
    sx = sum(x for x, _ in samples)
    sy = sum(y for _, y in samples)
    xx = sum(x*x for x, _ in samples)
    xy = sum(x*y for x, y in samples)
    den = n*xx-sx*sx
    if not den:
        raise ValueError('singular features')
    slope = (n*xy-sx*sy)/den
    return (sy-slope*sx)/n, slope


def expected_cost(success, fast, guard, recovery, baseline_cost):
    if not 0 <= success <= 1:
        raise ValueError('probability outside [0,1]')
    return guard + success*fast + (1-success)*(recovery+baseline_cost)
