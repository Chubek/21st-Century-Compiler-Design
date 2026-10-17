"""Sources 89--93: type errors, lexical scope, and differential execution."""
from pathlib import Path
import subprocess
import sys
import unittest
from fun import parse,Types,Machine,Lazy,VM,compile_code,optimize,Thunk


class Functional(unittest.TestCase):
    def compare(self,source,expected):
        e=parse(source);ty=Types();ty.infer(e)
        self.assertEqual(Machine().evaluate(e),expected)
        self.assertEqual(VM().execute(compile_code(e)),expected)
        self.assertEqual(Lazy().evaluate(e),expected)
        o=optimize(e);Types().infer(o)
        self.assertEqual(VM().execute(compile_code(o)),expected)
        self.assertEqual(Lazy().evaluate(o),expected)

    def test_examples_and_scope(self):
        for filename,answer in [('sum.fun',20),('factorial.fun',720)]:
            self.compare((Path(__file__).parent/filename).read_text(),answer)
        self.compare('(let x 11 (let f (lam y (add x y)) (let x 99 (app f 4))))',15)
        self.compare('(let id (lam x x) (if (app id true) (app id 17) 0))',17)
        self.compare('(case nil 8 (h t h))',8)
        self.compare('(case (cons 3 nil) 8 (h t h))',3)

    def test_inference_errors(self):
        for source in ['(app 1 2)','(lam x (app x x))','(add true 1)',
                       '(if true 1 false)','(cons 1 (cons true nil))',
                       '(lam f (if (app f true) (app f 1) 0))',
                       '(lam f (let g f (if (app g true) (app g 1) 0)))']:
            with self.subTest(source=source),self.assertRaises(TypeError):Types().infer(parse(source))
        with self.assertRaises(ValueError):parse('(letrec x x x)')
        for source in ['(lam x)','(add 1 2) 3','(',')','(case nil 0 (x x 1))']:
            with self.assertRaises(ValueError):parse(source)

    def test_many_closed_programs(self):
        for a in range(-6,7):
            for b in range(-6,7):
                for op in ['add','mul','eq']:
                    expected={'add':a+b,'mul':a*b,'eq':a==b}[op]
                    self.compare(f'(let f (lam x ({op} x {b})) (app f {a}))',expected)

    def test_lazy_sharing_and_divergence(self):
        lazy=Lazy();e=parse('(let x (add 18 5) (add x x))')
        self.assertEqual(lazy.evaluate(e),46);self.assertEqual(lazy.forces,1)
        t=Thunk('x',{});t.environment['x']=t
        with self.assertRaisesRegex(RuntimeError,'black hole'):Lazy().force(t)
        source='(letrec loop (lam x (app loop x)) (app (lam ignored 7) (app loop 0)))'
        e=parse(source);Types().infer(e)
        self.assertEqual(Lazy(100).evaluate(e),7)
        with self.assertRaises(RuntimeError):Machine(100).evaluate(e)
        e=parse('(letrec loop (lam x (app loop x)) (case (cons 4 (app loop 0)) 0 (h t h)))')
        Types().infer(e)
        self.assertEqual(Lazy(100).evaluate(e),4)
        with self.assertRaises(RuntimeError):Machine(100).evaluate(e)
        # Strict optimization must not erase a divergent operand of multiply by zero.
        e=parse('(letrec loop (lam x (app loop x)) (mul (app loop 0) 0))')
        self.assertEqual(optimize(e),e)

    def test_cli(self):
        here=Path(__file__).parent
        for mode in ('strict','vm','lazy'):
            p=subprocess.run([sys.executable,here/'fun.py',here/'sum.fun','--mode',mode],
                             capture_output=True,text=True,check=True)
            self.assertEqual(p.stdout.strip(),'20')


if __name__=='__main__':unittest.main(verbosity=2)
