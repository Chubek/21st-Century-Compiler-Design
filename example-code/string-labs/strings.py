"""Sources 77--80: Unicode keys, exact matching, and an implicit weighted FST.
Python 3.11+, standard library only. See README.md for limits and semantics.
"""
from dataclasses import dataclass
import argparse
import heapq
import json
from pathlib import Path
import unicodedata


def key(text):
    return unicodedata.normalize('NFC', unicodedata.normalize('NFC', text).casefold())


def prefix(pattern):
    pi = [0] * len(pattern)
    j = 0
    for i in range(1, len(pattern)):
        while j and pattern[i] != pattern[j]:
            j = pi[j-1]
        if pattern[i] == pattern[j]:
            j += 1
        pi[i] = j
    return pi


def find_all(text, pattern):
    if not pattern:
        return list(range(len(text)+1))
    pi, j, result = prefix(pattern), 0, []
    for i, char in enumerate(text):
        while j and char != pattern[j]:
            j = pi[j-1]
        if char == pattern[j]:
            j += 1
        if j == len(pattern):
            result.append(i+1-j)
            j = pi[j-1]
    return result


def distance(a, b):
    """Independent row-DP oracle; unit insertion/deletion/substitution."""
    row = list(range(len(b)+1))
    for i, x in enumerate(a, 1):
        new = [i]
        for j, y in enumerate(b, 1):
            new.append(min(row[j]+1, new[-1]+1, row[j-1]+(x != y)))
        row = new
    return row[-1]


class BudgetExceeded(RuntimeError):
    pass


@dataclass(frozen=True)
class Candidate:
    word: str
    cost: int
    frequency: int


class Spellchecker:
    """Trie identity transducer composed lazily with a unit-cost edit FST.
    Nodes have unique root paths, so one best cost per (input position,node)
    preserves every dictionary output. No minimized-DAG output conflation.
    """
    def __init__(self, entries):
        merged = {}
        for word, frequency in entries:
            if not isinstance(word, str) or not word or len(word) > 128:
                raise ValueError('dictionary word must contain 1..128 code points')
            if type(frequency) is not int or frequency <= 0:
                raise ValueError('frequency must be a positive integer')
            word = key(word)
            if len(word) > 128:
                raise ValueError('normalized word too long')
            merged[word] = merged.get(word, 0) + frequency
        self.edges, self.words, self.frequencies = [{}], [''], [0]
        for word, frequency in sorted(merged.items()):
            node = 0
            for char in word:
                if char not in self.edges[node]:
                    self.edges[node][char] = len(self.edges)
                    self.edges.append({})
                    self.words.append(self.words[node]+char)
                    self.frequencies.append(0)
                node = self.edges[node][char]
            self.frequencies[node] = frequency

    def suggest(self, text, radius=2, limit=10, budget=100000):
        if not isinstance(text, str) or len(text) > 128:
            raise ValueError('query must contain at most 128 code points')
        if type(radius) is not int or not 0 <= radius <= 3:
            raise ValueError('radius must be 0..3')
        if type(limit) is not int or limit <= 0 or type(budget) is not int or budget <= 0:
            raise ValueError('positive limit and state budget required')
        text = key(text)
        if len(text) > 128:
            raise ValueError('normalized query too long')
        best, queue, result, expanded = {(0, 0): 0}, [(0, 0, 0)], [], 0
        def offer(i, node, cost):
            if cost <= radius and cost < best.get((i, node), radius+1):
                best[i, node] = cost
                heapq.heappush(queue, (cost, i, node))
        while queue:
            cost, i, node = heapq.heappop(queue)
            if cost != best[i, node]:
                continue
            expanded += 1
            if expanded > budget:
                raise BudgetExceeded('no complete result: product-state budget exhausted')
            if i == len(text) and self.frequencies[node]:
                result.append(Candidate(self.words[node], cost, self.frequencies[node]))
            if i < len(text):
                offer(i+1, node, cost+1)  # input deletion: x:epsilon
            for char, target in self.edges[node].items():
                offer(i, target, cost+1)  # dictionary insertion: epsilon:y
                if i < len(text):
                    offer(i+1, target, cost+(text[i] != char))  # x:y
        return sorted(result, key=lambda c: (c.cost, -c.frequency, c.word))[:limit]


def main():
    parser = argparse.ArgumentParser(description='bounded Unicode FST spellchecker')
    parser.add_argument('dictionary', type=Path, help='UTF-8 TSV: word TAB positive frequency')
    parser.add_argument('word')
    parser.add_argument('--radius', type=int, default=2)
    parser.add_argument('--limit', type=int, default=10)
    parser.add_argument('--budget', type=int, default=100000)
    args = parser.parse_args()
    try:
        entries = []
        for line in args.dictionary.read_text(encoding='utf-8').splitlines():
            if line and not line.startswith('#'):
                word, frequency = line.split('\t')
                entries.append((word, int(frequency)))
        result = Spellchecker(entries).suggest(args.word, args.radius, args.limit, args.budget)
        print(json.dumps([vars(c) for c in result], ensure_ascii=False))
    except (ValueError, OSError, BudgetExceeded) as error:
        parser.exit(2, f'{error}\n')


if __name__ == '__main__':
    main()
