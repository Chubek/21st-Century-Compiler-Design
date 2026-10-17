"""Sources 77--80: exhaustive finite oracles and public CLI tests."""
from itertools import product
from pathlib import Path
import json
import subprocess
import sys
import unittest
from strings import key, prefix, find_all, distance, Spellchecker, BudgetExceeded


def words(alphabet, maximum):
    return [''.join(t) for n in range(maximum+1) for t in product(alphabet, repeat=n)]


class Strings(unittest.TestCase):
    def test_exact_exhaustive(self):
        for text in words('ab', 7):
            for pattern in words('ab', 4):
                expected = [i for i in range(len(text)+1) if text.startswith(pattern, i)]
                self.assertEqual(find_all(text, pattern), expected)
        self.assertEqual(prefix('ababa'), [0,0,1,2,3])
        self.assertEqual(find_all('ababababa', 'ababa'), [0,2,4])

    def test_fst_exhaustive(self):
        dictionary = words('ab', 4)[1:]
        checker = Spellchecker((w, i+1) for i, w in enumerate(dictionary))
        for query in words('abc', 4):
            for radius in range(4):
                expected = [(distance(query, w), -(i+1), w)
                            for i,w in enumerate(dictionary) if distance(query,w) <= radius]
                expected.sort()
                actual = checker.suggest(query, radius, 100, 10000)
                self.assertEqual([(c.cost, -c.frequency, c.word) for c in actual], expected)

    def test_unicode_and_ranking(self):
        self.assertEqual(key('CAFE\u0301'), key('café'))
        self.assertEqual(key('Straße'), 'strasse')
        checker = Spellchecker([('cat',30),('coat',20),('cot',10),('cut',5)])
        self.assertEqual([(c.word,c.cost) for c in checker.suggest('ct',1)],
                         [('cat',1),('cot',1),('cut',1)])
        self.assertEqual(distance('ct','coat'),2)
        merged = Spellchecker([('Straße',3),('STRASSE',4)])
        self.assertEqual(merged.suggest('strasse',0)[0].frequency,7)
        self.assertEqual(key(key('CAFE\u0301')),key('CAFE\u0301'))

    def test_limits_and_cli(self):
        checker = Spellchecker([('cat',1)])
        with self.assertRaises(BudgetExceeded): checker.suggest('cat',budget=1)
        for radius in (-1,4,True):
            with self.assertRaises(ValueError): checker.suggest('cat',radius)
        with self.assertRaises(ValueError): checker.suggest('x'*129)
        with self.assertRaises(ValueError): Spellchecker([('',1)])
        with self.assertRaises(ValueError): Spellchecker([('cat',0)])
        here=Path(__file__).resolve().parent
        p=subprocess.run([sys.executable,here/'strings.py',here/'dictionary.tsv','ct',
                          '--radius','1'],capture_output=True,text=True,check=True)
        self.assertEqual(json.loads(p.stdout)[0]['word'],'cat')
        p=subprocess.run([sys.executable,here/'strings.py',here/'dictionary.tsv','ct',
                          '--budget','1'],capture_output=True,text=True)
        self.assertEqual(p.returncode,2)
        self.assertEqual(p.stdout,'')


if __name__ == '__main__': unittest.main(verbosity=2)
