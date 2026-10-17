"""Recompute manuscript examples, including modeled extensions not in the C tools."""
import unittest


def relax():
    a=b=2
    history=[]
    while True:
        t=a+b+124; u=t+8
        da=t-a; db=u-a-b
        history.append((a,b,t,u,da,db))
        na=5 if not -128<=da<=127 else a
        nb=5 if not -128<=db<=127 else b
        if (na,nb)==(a,b):
            return history
        a,b=na,nb


def archives(order, group):
    members={'A':[('draw',{'draw'},{'blend'}),('helper',{'helper'},set())],
             'B':[('blend',{'blend'},{'helper'})]}
    selected=set(); defined=set(); needed={'draw'}; events=[]
    while True:
        old=set(selected)
        for archive in order:
            # Reach closure inside one archive before advancing the command.
            while True:
                before=set(selected)
                for name,defs,refs in members[archive]:
                    if name not in selected and defs & (needed-defined):
                        selected.add(name); defined |= defs; needed |= refs
                        events.append((name,needed-defined))
                if before==selected:
                    break
        if not group or selected==old:
            return selected,needed-defined,events


def reachable(edges, roots):
    live=set(roots); todo=list(roots)
    while todo:
        for target in edges.get(todo.pop(),()):
            if target not in live:
                live.add(target); todo.append(target)
    return live


class WorkedExamples(unittest.TestCase):
    def test_widening_feedback(self):
        self.assertEqual(relax(),[(2,2,128,136,126,132),
                                 (2,5,131,139,129,132),
                                 (5,5,134,142,129,132)])

    def test_archive_order_and_group(self):
        self.assertEqual(archives(['A','B'],False)[1],{'helper'})
        self.assertEqual(archives(['B','A'],False)[1],{'blend'})
        for order in (['A','B'],['B','A']):
            chosen,unresolved,_=archives(order,True)
            self.assertEqual(chosen,{'draw','blend','helper'})
            self.assertEqual(unresolved,set())

    def test_section_reachability(self):
        self.assertEqual(reachable({'entry':['a'],'a':['b'],'b':['a'],'orphan':['b']},['entry']),
                         {'entry','a','b'})

    def test_layout_and_numeric_answers(self):
        up=lambda x,a:(x+a-1)&~(a-1)
        self.assertEqual((up(31,16),up(31,16)+6),(32,38))
        self.assertEqual((up(8,16)+32,up(8,32)+32),(48,64))
        self.assertEqual([up(x,16)-x for x in (29,30,32)],[3,2,0])
        self.assertEqual(0x501080-4-0x501011,0x6b)
        self.assertEqual((0x401005-4-0x40100d,0x401020-4-0x401012),(-12,10))
        self.assertEqual(0xc0+8*3+2,0xda)
        self.assertEqual(up(4090,16),4096)


if __name__=='__main__':
    unittest.main(verbosity=2)
