"""Run: python3 -m unittest discover -s example-code/architecture-labs -v."""
from itertools import product
from pathlib import Path
import os
import platform
import subprocess
import tempfile
import unittest
from labs import (Queries, EGraph, eval_term, Effect, commute, memory_program,
                  lift, execute_lifted, add8, AddSite, interpret, specialize,
                  run_bytecode, baseline, optimized, materialize, Runtime,
                  ArrayHandle, placement, fit_line, expected_cost)


class ArchitectureLabs(unittest.TestCase):
    def test_incremental_early_cutoff(self):
        q = Queries({'body': 7, 'flag': True}, {
            'kind': lambda get: 'int' if type(get('body')) is int else 'text',
            'client': lambda get: get('kind') + '-call',
            'select': lambda get: get('body') if get('flag') else 0})
        self.assertEqual(q.get('client'), 'int-call')
        q.edit('body', 8)
        self.assertEqual(q.get('client'), 'int-call')
        self.assertEqual((q.calls['kind'], q.calls['client']), (2, 1))
        q.get('select'); q.edit('flag', False)
        self.assertEqual(q.get('select'), 0)
        self.assertEqual(q.cache['select'].dependencies, ('flag',))
        q.edit('body', 'hello')
        self.assertEqual(q.get('client'), 'text-call')
        self.assertEqual(q.get('select'), 0)
        self.assertEqual(q.calls['select'], 2)

    def test_dependency_order_and_cycle(self):
        q = Queries({'flag': True, 'body': 3}, {
            'select': lambda get: get('body') if get('flag') else 0})
        q.get('select')
        q.edit('flag', False)
        # A stale dependency must not be forced after changed control input.
        del q.inputs['body']
        self.assertEqual(q.get('select'), 0)
        c = Queries({}, {'a': lambda get: get('b'), 'b': lambda get: get('a')})
        with self.assertRaises(RuntimeError):
            c.get('a')
        self.assertEqual(c.active, [])

    def test_incremental_matches_fresh(self):
        providers = {'parity': lambda get: get('x') % 2,
                     'result': lambda get: get('parity') + get('y')}
        q = Queries({'x': 0, 'y': 0}, providers)
        for x, y in product(range(10), repeat=2):
            q.edit('x', x); q.edit('y', y)
            self.assertEqual(q.get('result'), Queries({'x': x, 'y': y}, providers).get('result'))

    def test_egraph_identities_and_congruence(self):
        e = EGraph(); x = e.add('var'); zero = e.add('const', payload=0)
        one = e.add('const', payload=1)
        root = e.add('mul', (e.add('add', (x, zero)), one))
        before = e.extract(root)[1]
        self.assertTrue(e.saturate())
        cost, after = e.extract(root)
        self.assertEqual(cost, 1)
        for value in range(256):
            self.assertEqual(eval_term(before, value), eval_term(after, value))
        a = e.add('add', (x, one)); b = e.add('add', (root, one))
        e.rebuild(); self.assertEqual(e.find(a), e.find(b))
        for c in range(256):
            g = EGraph(); a = g.add('const', payload=c); b = g.add('const', payload=13)
            r = g.add('mul', (a, b)); g.saturate()
            self.assertEqual(eval_term(g.extract(r)[1], 0), c*13 % 256)

    def test_egraph_budget_and_unfounded_cycle(self):
        e = EGraph(); x = e.add('var'); z = e.add('const', payload=0)
        r = e.add('add', (x,z))
        self.assertFalse(e.saturate(rounds=0))
        self.assertEqual(eval_term(e.extract(r)[1], 17), 17)
        # Artificial cyclic class without a base case has no finite term.
        e = EGraph(); e.parent=[0]; e.nodes=[(0,('add',None,(0,0)))]
        with self.assertRaises(ValueError):
            e.extract(0)

    def test_effects_and_aliases(self):
        for p, q in product(range(4), repeat=2):
            a, b = Effect(writes=frozenset([p])), Effect(writes=frozenset([q]))
            self.assertEqual(commute(a,b), p != q)
            self.assertEqual(memory_program(p,q) == memory_program(p,q,True), p != q)
        self.assertFalse(commute(Effect(may_trap=True), Effect()))
        self.assertFalse(commute(Effect(may_diverge=True), Effect()))

    @unittest.skipUnless(platform.machine() == 'x86_64', 'hardware oracle requires x86-64')
    def test_lift_against_hardware(self):
        bits = {'CF':0, 'PF':2, 'AF':4, 'ZF':6, 'SF':7, 'OF':11}
        with tempfile.TemporaryDirectory(dir='/tmp/opencode', prefix='arch-flags-') as temp:
            for cc in ('gcc', 'clang'):
                for san in (False, True):
                    out = Path(temp)/(cc+str(san))
                    args = [cc, '-std=c11', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                            '-mno-red-zone', str(Path(__file__).with_name('flags.c')), '-o', str(out)]
                    if san:
                        args += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
                    subprocess.run(args, check=True, capture_output=True, timeout=60)
                    p = subprocess.run([out], check=True, capture_output=True, text=True, timeout=30,
                                       env={**os.environ, 'ASAN_OPTIONS':'detect_leaks=1'})
                    self.assertNotIn('runtime error', p.stderr)
                    self.assertEqual(len(p.stdout.splitlines()), 65536)
                    for line in p.stdout.splitlines():
                        a,b,result,flags = map(int,line.split())
                        value, f = execute_lifted(lift(bytes([0x04,b])), 0x1200+a, {})
                        self.assertEqual(value, 0x1200+result)
                        self.assertEqual(f, {name:(flags>>bit)&1 for name,bit in bits.items()})

    def test_lift_partial_registers_and_rejection(self):
        r, flags = execute_lifted(lift(bytes([0xb0,0xff,0x04,1])), 0x12345600, {})
        self.assertEqual(r, 0x12345600)
        self.assertEqual((flags['CF'], flags['ZF'], flags['OF']), (1,1,0))
        self.assertEqual(add8(127,1)[1]['OF'],1)
        for data in (b'\x04', b'\x90', b'\x04\x01\xff'):
            with self.assertRaises(ValueError):
                lift(data)

    def test_mov_xor_contract(self):
        initial = dict(CF=1, PF=0, AF=1, ZF=0, SF=1, OF=1)
        for a, b in product(range(256), repeat=2):
            r, f = execute_lifted(lift(bytes([0xb0, b])), 0xAB00+a, initial)
            self.assertEqual((r, f), (0xAB00+b, initial))
            r, f = execute_lifted(lift(bytes([0x34, b])), 0xAB00+a, initial)
            bits = [(a >> k & 1) != (b >> k & 1) for k in range(8)]
            expected = sum(int(bit) * 2**k for k, bit in enumerate(bits))
            self.assertEqual(r, 0xAB00+expected)
            self.assertEqual(f, dict(CF=0, OF=0, AF=None, ZF=int(not any(bits)),
                                     SF=int(bits[7]), PF=int(sum(bits) % 2 == 0)))

    def test_quickening_and_staging(self):
        site=AddSite()
        self.assertEqual(site(3,4),7); self.assertEqual(site.state,'int')
        self.assertEqual(site('a','b'),'ab'); self.assertEqual(site.state,'generic')
        self.assertEqual(site(8,9),17); self.assertEqual(site.misses,1)
        with self.assertRaises(TypeError):
            site(True,2)
        for k in range(-8,9):
            expr=('mul',('add',('input',),('const',k)),('const',3))
            for x in range(-100,101):
                self.assertEqual(interpret(expr,x),run_bytecode(specialize(expr),x))

    def test_all_deopt_and_osr_positions(self):
        for length in range(5):
            for values in product((-129,-1,0,7,127,'bad'), repeat=length):
                for entry in range(length+1):
                    self.assertEqual(optimized(values,entry),baseline(values))

    def test_virtual_identity(self):
        o=materialize({'a':{'next':('ref','b')}, 'b':{'next':('ref','a')}})
        self.assertIs(o['a']['next']['next'],o['a'])

    def test_publication_and_reclamation(self):
        r=Runtime(); old=r.acquire(); request=r.epoch
        r.edit()
        self.assertFalse(r.publish(request,'stale'))
        r.collect(); self.assertIn(old,r.versions)
        r.release(old); r.collect(); self.assertNotIn(old,r.versions)
        self.assertEqual(r.current.body,'generic')
        self.assertTrue(r.publish(r.epoch,'specialized'))

    def test_representation_migration(self):
        for values in product((0,1,4),repeat=5):
            h=ArrayHandle(values); alias=h
            for sparse in (True,False,True):
                h.migrate(sparse)
                self.assertEqual(tuple(alias.get(i) for i in range(5)),values)
                self.assertIs(alias,h)
            with self.assertRaises(IndexError):
                h.get(5)

    def test_cost_and_placement(self):
        intercept,slope=fit_line([(1,7),(2,10),(3,13),(4,16)])
        self.assertEqual((intercept,slope),(4,3))
        self.assertEqual(intercept+slope*5,19)
        self.assertAlmostEqual(expected_cost(.9,4,1,20,10),7.6)
        self.assertAlmostEqual(expected_cost(.7,4,1,20,10),12.8)
        with self.assertRaises(ValueError):
            expected_cost(1.1,4,1,20,10)
        weights=[[0,9,1],[9,0,8],[1,8,0]]
        self.assertEqual(placement(weights),(19,(0,1,2)))
        # CPU=4n, GPU=120+n including transfer/launch: GPU wins only at n>40.
        self.assertEqual(4*40,120+40)
        self.assertGreater(4*41,120+41)

    def test_worked_examples_and_solutions(self):
        q = Queries({'x':2, 'y':10}, {'parity':lambda g:g('x') % 2,
                                     'client':lambda g:g('parity')+g('y')})
        self.assertEqual(q.get('client'),10)
        q.edit('x',4); self.assertEqual(q.get('client'),10)
        self.assertEqual((q.calls['parity'],q.calls['client']),(2,1))
        q.edit('y',11); self.assertEqual(q.get('client'),11)
        self.assertEqual((q.calls['parity'],q.calls['client']),(2,2))
        e=EGraph(); a=e.add('const',payload=250); b=e.add('const',payload=13)
        s=e.add('add',(a,b)); p=e.add('mul',(s,b)); e.saturate()
        self.assertEqual(e.extract(s),(1,('const',7,())))
        self.assertEqual(e.extract(p),(1,('const',91,())))
        self.assertEqual(memory_program(1,2),(9,((1,9),(2,4))))
        self.assertEqual(memory_program(1,1),(4,((1,4),)))
        self.assertEqual(memory_program(1,1,True),(9,((1,9),)))
        r,_=execute_lifted(lift(bytes([0xb0,7,0x04,9])),0xabcd12,{})
        self.assertEqual(r,0xabcd10)
        self.assertEqual(run_bytecode(specialize(('mul',('add',('input',),('const',2)),
                                                 ('const',4))),3),20)
        self.assertEqual(optimized([7,120,1]),('ok',3,128,((0,7),(1,127),(2,128))))
        self.assertEqual(optimized([7,'bad',4],1),('type-error',1,7,((0,7),)))
        h=ArrayHandle([0,4,0,1,0]); h.migrate(True)
        self.assertEqual(h.storage,{1:4,3:1})
        self.assertAlmostEqual(expected_cost(21/26,4,1,20,10),10)
        self.assertEqual([sum(x for x,y in [(1,7),(2,10),(3,13),(4,16)]),
                          sum(y for x,y in [(1,7),(2,10),(3,13),(4,16)]),
                          sum(x*x for x,y in [(1,7),(2,10),(3,13),(4,16)]),
                          sum(x*y for x,y in [(1,7),(2,10),(3,13),(4,16)])],
                         [10,46,30,130])
        costs = {}
        from itertools import permutations
        for order in permutations('ABC'):
            pos={v:i for i,v in enumerate(order)}
            costs[''.join(order)]=sum(w*abs(pos[a]-pos[b]) for a,b,w in
                                      [('A','B',9),('B','C',8),('A','C',1)])
        self.assertEqual(costs,dict(ABC=19,ACB=27,BAC=26,BCA=27,CAB=26,CBA=19))
        self.assertEqual([(4*n,120+n) for n in (20,40,80)],[(80,140),(160,160),(320,200)])


if __name__ == '__main__':
    unittest.main(verbosity=2)
