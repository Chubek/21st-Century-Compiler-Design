"""Recompute source chapters 49–51 tables and compare against language oracles."""
import itertools
import unittest


def facts(rules, start):
    nullable = set()
    first = {n: set() for n in rules}
    def sequence(rhs):
        result = set()
        for x in rhs:
            result |= first[x] if x in rules else {x}
            if x not in nullable:
                return result, False
        return result, True
    while True:
        before = (len(nullable), sum(map(len, first.values())))
        for n, alternatives in rules.items():
            for rhs in alternatives:
                f, empty = sequence(rhs)
                first[n] |= f
                if empty:
                    nullable.add(n)
        if before == (len(nullable), sum(map(len, first.values()))):
            break
    follow = {n: set() for n in rules}
    follow[start].add('$')
    while True:
        before = sum(map(len, follow.values()))
        for n, alternatives in rules.items():
            for rhs in alternatives:
                for i, x in enumerate(rhs):
                    if x not in rules:
                        continue
                    f, empty = sequence(rhs[i + 1:])
                    follow[x] |= f
                    if empty:
                        follow[x] |= follow[n]
        if before == sum(map(len, follow.values())):
            break
    table = {}
    for n, alternatives in rules.items():
        for rhs in alternatives:
            f, empty = sequence(rhs)
            for token in f | (follow[n] if empty else set()):
                table.setdefault((n, token), []).append(rhs)
    return nullable, first, follow, table


def predictive(table, rules, tokens):
    stack, position = ['$', 'S'], 0
    stream = (*tokens, '$')
    while stack:
        x = stack.pop()
        if x in rules:
            entry = table.get((x, stream[position]), [])
            if len(entry) != 1:
                return False
            stack.extend(reversed(entry[0]))
        else:
            if x != stream[position]:
                return False
            position += 1
    return position == len(stream)


def chart(rules, tokens):
    rules = {'@': [('S',)], **rules}
    columns = [set() for _ in range(len(tokens) + 1)]
    columns[0].add(('@', ('S',), 0, 0))
    for j, column in enumerate(columns):
        while True:
            old = set(column)
            for lhs, rhs, dot, origin in old:
                if dot < len(rhs) and rhs[dot] in rules:
                    for alt in rules[rhs[dot]]:
                        column.add((rhs[dot], alt, 0, j))
                elif dot == len(rhs):
                    for a, b, d, h in list(columns[origin]):
                        if d < len(b) and b[d] == lhs:
                            column.add((a, b, d + 1, h))
            if old == column:
                break
        if j < len(tokens):
            for a, b, d, h in column:
                if d < len(b) and b[d] == tokens[j]:
                    columns[j + 1].add((a, b, d + 1, h))
    return ('@', ('S',), 1, 0) in columns[-1], columns


class FrontEndExamples(unittest.TestCase):
    def test_subset_and_minimization(self):
        def close(states):
            return frozenset(set(states) | ({2} if 1 in states else set()))
        edges = {(0, 'a'): {1}, (2, 'b'): {2}, (2, 'c'): {2}, (2, 'd'): {3}}
        states = [close({0})]
        transitions = {}
        for state in states:
            for a in 'abcd':
                nxt = close(set().union(*(edges.get((q, a), set()) for q in state)))
                if nxt not in states:
                    states.append(nxt)
                transitions[state, a] = nxt
        self.assertEqual(set(states), {frozenset(x) for x in ({0}, {1, 2}, {2}, {3}, set())})
        partition = {s: int(3 in s) for s in states}
        while True:
            signatures = {s: (partition[s], *(partition[transitions[s, a]] for a in 'abcd')) for s in states}
            groups = {key: i for i, key in enumerate(dict.fromkeys(signatures.values()))}
            new = {s: groups[signatures[s]] for s in states}
            if len(set(new.values())) == len(set(partition.values())):
                break
            partition = new
        self.assertEqual(len(set(new.values())), 4)
        self.assertEqual(new[frozenset({1, 2})], new[frozenset({2})])

    def test_predictive_facts_and_language(self):
        rules = {'S': [('[', 'I', ']')], 'I': [('id', 'T'), ()], 'T': [(',', 'id', 'T'), ()]}
        nullable, first, follow, table = facts(rules, 'S')
        self.assertEqual(nullable, {'I', 'T'})
        self.assertEqual(first, {'S': {'['}, 'I': {'id'}, 'T': {','}})
        self.assertEqual(follow, {'S': {'$'}, 'I': {']'}, 'T': {']'}})
        self.assertEqual(len(table), 5)
        valid = {('[', ']'), ('[', 'id', ']'), ('[', 'id', ',', 'id', ']'),
                 ('[', 'id', ',', 'id', ',', 'id', ']')}
        for n in range(8):
            for tokens in itertools.product(('[', ']', 'id', ','), repeat=n):
                self.assertEqual(predictive(table, rules, tokens), tokens in valid)

    def test_nullable_conflict(self):
        _, _, _, table = facts({'S': [('A', 'id')], 'A': [('id',), ()]}, 'S')
        self.assertEqual(len(table['A', 'id']), 2)

    def test_chart_left_recursion_and_nullable(self):
        rules = {'S': [('S', 'S'), ('a',)]}
        for n in range(8):
            for word in itertools.product('ab', repeat=n):
                self.assertEqual(chart(rules, word)[0], bool(word) and set(word) == {'a'})
        accepted, columns = chart({'S': [('A', 'B')], 'A': [()], 'B': [()]}, ())
        self.assertTrue(accepted)
        self.assertEqual(len(columns[0]), 7)


if __name__ == '__main__':
    unittest.main()
