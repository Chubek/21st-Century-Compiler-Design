"""Sources 81--88: original finite rewrite examples and independent checks."""
import unittest
from itertools import product
from rewrite import (Var,term,apply,match,unify,normalize,replay,critical_pairs,
                     string_normalize,Node,unfold,redirect_add_zero,delete_vertices,select_tree)

x,y=Var('x'),Var('y')
z=term('zero')
rules=[(term('add',x,z),x),(term('mul',x,term('one')),x)]


class Rewriting(unittest.TestCase):
    def test_match_and_unify(self):
        p=term('pair',x,x)
        self.assertIsNone(match(p,term('pair',z,term('one'))))
        self.assertEqual(match(p,term('pair',z,z)),{x:z})
        self.assertIsNone(unify(x,term('f',x)))
        self.assertIsNone(unify(term('pair',x,y),term('pair',y,term('f',x))))
        env=unify(term('f',x,term('g',y)),term('f',term('g',z),term('g',z)))
        self.assertEqual(apply(x,env),term('g',z))
        self.assertEqual(apply(y,env),z)
        terms=[x,y,z,term('one'),term('f',x),term('f',z),term('pair',x,y)]
        for a,b in product(terms,repeat=2):
            env=unify(a,b)
            if env is not None: self.assertEqual(apply(a,env),apply(b,env))

    def test_normalization_certificates(self):
        self.assertEqual(normalize(term('add',x,z),rules)[0],x)
        # A subject variable with the same ID as a rule variable is not recursively rebound.
        self.assertEqual(normalize(term('add',term('f',x),z),rules)[0],term('f',x))
        a=term('v')
        for depth in range(50):
            t=a
            for _ in range(depth): t=term('mul',term('add',t,z),term('one'))
            result,trace,complete=normalize(t,rules)
            self.assertTrue(complete);self.assertEqual(result,a)
            self.assertEqual(replay(t,rules,trace),result)
            self.assertEqual(len(trace),2*depth)
            self.assertEqual(normalize(t,rules,innermost=True)[0],a)
        loop=[(term('f',x),term('f',x))]
        self.assertFalse(normalize(term('f',z),loop,3)[2])
        with self.assertRaises(ValueError): normalize(z,[(x,z)])
        with self.assertRaises(ValueError): replay(z,rules,[(-1,(),{})])
        with self.assertRaises(ValueError): replay(term('add',term('v'),z),rules,[(0,(-1,),{})])
        with self.assertRaises(ValueError): replay(term('add',term('v'),z),rules,[(0,(),{x:z})])

    def test_peaks_and_strings(self):
        peak=[(term('f',term('a')),term('b')),(term('a'),term('c'))]
        pairs=critical_pairs(peak)
        self.assertTrue(any(set(pair)=={term('b'),term('f',term('c'))} for pair in pairs))
        self.assertEqual(critical_pairs(rules),[])
        # Weighted interpretation for the repaired peak, not plain node count.
        def weight(t):
            return {'a':2,'b':0,'c':1}[t[0]] if len(t)==1 else 1+weight(t[1])
        repaired=peak+[(term('f',term('c')),term('b'))]
        self.assertTrue(all(weight(l)>weight(r) for l,r in repaired))
        self.assertEqual(string_normalize('aaa',[('aa','b')]),('ba',True))
        self.assertEqual(string_normalize('ababa',[('ab','a'),('aa','a')]),('a',True))
        self.assertFalse(string_normalize('a',[('a','aa')],5)[1])
        with self.assertRaises(ValueError):string_normalize('a',[('','a')])

    def test_graph_sharing_and_dangling(self):
        heap=[Node('v'),Node('zero'),Node('add',(0,1)),Node('pair',(2,2))]
        self.assertEqual(unfold(heap,3),term('pair',term('add',term('v'),z),term('add',term('v'),z)))
        self.assertTrue(redirect_add_zero(heap,2))
        self.assertEqual(unfold(heap,3),term('pair',term('v'),term('v')))
        with self.assertRaises(ValueError):unfold([Node('ind',(0,))],0)
        with self.assertRaises(ValueError):delete_vertices({0,1},{(0,1)},{1})
        self.assertEqual(delete_vertices({0,1},set(),{1}),({0},set()))

    def test_selection(self):
        expression=term('add',term('mul',term('a'),term('b')),term('c'))
        self.assertEqual(select_tree(expression),(3,('madd','a','b','c')))
        # Independent enumeration of both legal root covers for this shape.
        self.assertEqual(select_tree(expression)[0],min(2+3,3))


if __name__=='__main__':unittest.main(verbosity=2)
