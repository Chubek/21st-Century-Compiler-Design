"""Sources 81--88: finite first-order rewriting and explicit graph operations.
Python 3.11+, standard library. Immutable terms; no binding in object terms.
"""
from dataclasses import dataclass


@dataclass(frozen=True)
class Var:
    name: str


def term(symbol, *children):
    return (symbol, *children)


def variables(t):
    return {t} if isinstance(t, Var) else set().union(*(variables(c) for c in t[1:]))


def apply(t, substitution):
    if isinstance(t, Var):
        return apply(substitution[t], substitution) if t in substitution else t
    return (t[0], *(apply(c, substitution) for c in t[1:]))


def instantiate(t, substitution):
    """Simultaneous matcher substitution; subject variables are not rebound."""
    if isinstance(t, Var): return substitution.get(t,t)
    return (t[0], *(instantiate(c,substitution) for c in t[1:]))


def match(pattern, subject, env=None):
    env = {} if env is None else dict(env)
    todo = [(pattern, subject)]
    while todo:
        p, s = todo.pop()
        if isinstance(p, Var):
            if p in env and env[p] != s: return None
            env[p] = s
        elif isinstance(s, Var) or len(p) != len(s) or p[0] != s[0]:
            return None
        else: todo.extend(zip(p[1:], s[1:]))
    return env


def unify(left, right):
    env, todo = {}, [(left, right)]
    while todo:
        a,b = (apply(x,env) for x in todo.pop())
        if a == b: continue
        if not isinstance(a,Var) and isinstance(b,Var): a,b=b,a
        if isinstance(a,Var):
            if a in variables(b): return None
            env[a]=b
        elif len(a)!=len(b) or a[0]!=b[0]: return None
        else: todo.extend(zip(a[1:],b[1:]))
    return {x:apply(t,env) for x,t in env.items()}


def positions(t):
    yield ()
    if not isinstance(t,Var):
        for i,c in enumerate(t[1:]):
            for p in positions(c): yield (i,)+p


def at(t,p):
    for i in p:
        if isinstance(t,Var) or type(i) is not int or not 0<=i<len(t)-1:
            raise ValueError('invalid term position')
        t=t[i+1]
    return t


def replace(t,p,r):
    if not p: return r
    children=list(t[1:]); i=p[0]
    children[i]=replace(children[i],p[1:],r)
    return (t[0],*children)


def check_rule(rule):
    l,r=rule
    if isinstance(l,Var) or not variables(r)<=variables(l):
        raise ValueError('invalid first-order rule')


def step(t,rules,innermost=False):
    ps=list(positions(t))
    if innermost: ps.sort(key=len,reverse=True)
    for p in ps:
        for index,rule in enumerate(rules):
            check_rule(rule)
            env=match(rule[0],at(t,p))
            if env is not None:
                result=replace(t,p,instantiate(rule[1],env))
                return result,(index,p,env)
    return None


def normalize(t,rules,fuel=1000,innermost=False):
    if fuel<0: raise ValueError('negative fuel')
    trace=[]
    for _ in range(fuel):
        result=step(t,rules,innermost)
        if result is None: return t,trace,True
        t,event=result; trace.append(event)
    return t,trace,step(t,rules,innermost) is None


def replay(t,rules,trace):
    for index,p,env in trace:
        if type(index) is not int or not 0<=index<len(rules):
            raise ValueError('invalid rule index')
        l,r=rules[index]
        if match(l,at(t,p))!=env: raise ValueError('bad rewrite certificate')
        t=replace(t,p,instantiate(r,env))
    return t


def critical_pairs(rules):
    result=[]
    for i,(l,r) in enumerate(rules):
        check_rule((l,r))
        rename={v:Var(f'left:{i}:{v.name}') for v in variables(l)}
        a,b=instantiate(l,rename),instantiate(r,rename)
        for j,(s,t) in enumerate(rules):
            rename={v:Var(f'right:{j}:{v.name}') for v in variables(s)}
            s,t=instantiate(s,rename),instantiate(t,rename)
            for p in positions(s):
                if isinstance(at(s,p),Var): continue
                env=unify(a,at(s,p))
                if env is not None:
                    pair=(apply(replace(s,p,b),env),apply(t,env))
                    if pair[0]!=pair[1]: result.append(pair)
    return result


def string_normalize(text,rules,fuel=100):
    if any(not l for l,r in rules): raise ValueError('empty left side')
    for _ in range(fuel):
        choices=[(i,k,l,r) for k,(l,r) in enumerate(rules)
                 for i in range(len(text)-len(l)+1) if text.startswith(l,i)]
        if not choices: return text,True
        i,k,l,r=min(choices)
        text=text[:i]+r+text[i+len(l):]
    return text,not any(l in text for l,r in rules)


def polynomial_weight(t):
    """All constants weight 1; every operator weight 1 plus child weights."""
    return 1 if isinstance(t,Var) else 1+sum(polynomial_weight(c) for c in t[1:])


@dataclass
class Node:
    op: str
    children: tuple = ()


def unfold(heap,root,active=frozenset()):
    if root in active: raise ValueError('cycle has no finite tree unfolding')
    n=heap[root]
    if n.op=='ind': return unfold(heap,n.children[0],active|{root})
    return (n.op,*(unfold(heap,c,active|{root}) for c in n.children))


def redirect_add_zero(heap,root):
    """Shared update for pure total arithmetic only; checked finite heap shape."""
    n=heap[root]
    if n.op=='add' and len(n.children)==2 and heap[n.children[1]].op=='zero':
        heap[root]=Node('ind',(n.children[0],)); return True
    return False


def delete_vertices(vertices,edges,deleted):
    """DPO-style dangling check for a supplied injective deletion match.
    Incident edges must be explicitly selected for deletion in a full rule;
    this restricted operation selects none, hence forbids every dangling edge.
    """
    if not deleted<=vertices: raise ValueError('unknown vertex')
    if any(a in deleted or b in deleted for a,b in edges):
        raise ValueError('dangling edge')
    return vertices-deleted,set(edges)


def select_tree(t):
    """Minimum additive tree cover: leaf 0, add 2, multiply 3, fused madd 3."""
    if len(t)==1: return 0,t[0]
    children=[select_tree(c) for c in t[1:]]
    cost=(2 if t[0]=='add' else 3)+sum(c for c,s in children)
    code=(t[0],*(s for c,s in children))
    if t[0]=='add' and t[1][0]=='mul':
        a,b,c=t[1][1],t[1][2],t[2]
        options=[select_tree(x) for x in (a,b,c)]
        fused=3+sum(v for v,s in options)
        if fused<cost: return fused,('madd',*(s for v,s in options))
    return cost,code
