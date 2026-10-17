"""Sources 89--93: a small functional language, HM inference, and two machines.
Python 3.11+, no packages. S-expression syntax, finite budgets; see README.md.
"""
from dataclasses import dataclass
import argparse
from pathlib import Path
import re


def parse(source):
    tokens=re.findall(r'\(|\)|[^\s()]+',source)
    if len(tokens)>10000: raise ValueError('token budget')
    index=0
    def read(depth=0):
        nonlocal index
        if depth>100 or index>=len(tokens):raise ValueError('syntax or nesting limit')
        token=tokens[index]; index+=1
        if token=='(':
            items=[]
            while index<len(tokens) and tokens[index]!=')':items.append(read(depth+1))
            if index>=len(tokens):raise ValueError('missing closing parenthesis')
            index+=1
            if not items:raise ValueError('empty application')
            return tuple(items)
        if token==')':raise ValueError('unexpected closing parenthesis')
        if re.fullmatch(r'-?\d+',token):return int(token)
        if token in ('true','false'):return token=='true'
        return token
    result=read()
    if index!=len(tokens):raise ValueError('trailing expression')
    validate(result)
    return result


ARITIES={'lam':3,'app':3,'let':4,'letrec':4,'if':4,'add':3,'mul':3,
         'eq':3,'cons':3,'case':4}
RESERVED=set(ARITIES)|{'nil','true','false'}


def name(x):
    return isinstance(x,str) and bool(re.fullmatch(r'[A-Za-z_][A-Za-z_0-9]*',x)) and x not in RESERVED


def validate(e):
    if type(e) in (int,bool):return
    if isinstance(e,str):
        if e!='nil' and not name(e):raise ValueError('invalid identifier')
        return
    if not isinstance(e,tuple) or not e or e[0] not in ARITIES or len(e)!=ARITIES[e[0]]:
        raise ValueError('invalid expression shape')
    op=e[0]
    if op=='lam':
        if not name(e[1]):raise ValueError('invalid binder')
        validate(e[2])
    elif op in ('let','letrec'):
        if not name(e[1]):raise ValueError('invalid binder')
        if op=='letrec' and not (isinstance(e[2],tuple) and e[2][0]=='lam'):
            raise ValueError('letrec requires a lambda right-hand side')
        validate(e[2]);validate(e[3])
    elif op=='case':
        validate(e[1]);validate(e[2]);p=e[3]
        if not isinstance(p,tuple) or len(p)!=3 or not name(p[0]) or not name(p[1]) or p[0]==p[1]:
            raise ValueError('case requires distinct head/tail binders')
        validate(p[2])
    else:
        for c in e[1:]:validate(c)


@dataclass(frozen=True)
class TV:
    id: int


class Types:
    def __init__(self):self.next=0;self.sub={}
    def fresh(self):
        t=TV(self.next);self.next+=1;return t
    def resolve(self,t):
        if isinstance(t,TV):
            if t in self.sub:
                self.sub[t]=self.resolve(self.sub[t]);return self.sub[t]
            return t
        return tuple(self.resolve(x) for x in t) if isinstance(t,tuple) else t
    def free(self,t):
        t=self.resolve(t)
        if isinstance(t,TV):return {t}
        return set().union(*(self.free(c) for c in t[1:])) if isinstance(t,tuple) else set()
    def unify(self,a,b):
        a,b=self.resolve(a),self.resolve(b)
        if a==b:return
        if not isinstance(a,TV) and isinstance(b,TV):a,b=b,a
        if isinstance(a,TV):
            if a in self.free(b):raise TypeError('infinite type (occurs check)')
            self.sub[a]=b;return
        if not isinstance(a,tuple) or not isinstance(b,tuple) or len(a)!=len(b) or a[0]!=b[0]:
            raise TypeError('type mismatch')
        for x,y in zip(a[1:],b[1:]):self.unify(x,y)
    def generalize(self,t,env):
        excluded=set()
        for quantified,body in env.values():excluded|=self.free(body)-set(quantified)
        return self.free(t)-excluded,self.resolve(t)
    def instantiate(self,scheme):
        qs,t=scheme;mapping={q:self.fresh() for q in qs}
        def walk(t):
            t=self.resolve(t)
            if isinstance(t,TV):return mapping.get(t,t)
            return tuple(walk(x) for x in t) if isinstance(t,tuple) else t
        return walk(t)
    def infer(self,e,env=None):
        env={} if env is None else env
        if type(e) is bool:return 'Bool'
        if type(e) is int:return 'Int'
        if e=='nil':return ('List',self.fresh())
        if isinstance(e,str):
            if e not in env:raise TypeError('unbound variable: '+e)
            return self.instantiate(env[e])
        op=e[0]
        if op=='lam':
            a=self.fresh();b=self.infer(e[2],dict(env,**{e[1]:(set(),a)}))
            return self.resolve(('->',a,b))
        if op=='app':
            f=self.infer(e[1],env);a=self.infer(e[2],env);r=self.fresh()
            self.unify(f,('->',a,r));return self.resolve(r)
        if op in ('let','letrec'):
            local=dict(env)
            if op=='letrec':
                placeholder=self.fresh();local[e[1]]=(set(),placeholder)
                value=self.infer(e[2],local);self.unify(placeholder,value)
            else:value=self.infer(e[2],env)
            local=dict(env);local[e[1]]=self.generalize(value,env)
            return self.infer(e[3],local)
        if op=='if':
            self.unify(self.infer(e[1],env),'Bool')
            a=self.infer(e[2],env);b=self.infer(e[3],env);self.unify(a,b)
            return self.resolve(a)
        if op in ('add','mul','eq'):
            self.unify(self.infer(e[1],env),'Int');self.unify(self.infer(e[2],env),'Int')
            return 'Bool' if op=='eq' else 'Int'
        if op=='cons':
            a=self.infer(e[1],env);b=self.infer(e[2],env);self.unify(b,('List',a))
            return self.resolve(('List',a))
        if op=='case':
            a=self.fresh();self.unify(self.infer(e[1],env),('List',a))
            empty=self.infer(e[2],env);h,t,body=e[3]
            local=dict(env);local[h]=(set(),a);local[t]=(set(),('List',a))
            self.unify(empty,self.infer(body,local));return self.resolve(empty)
        raise ValueError(op)


@dataclass
class Cell:
    value: object = None


@dataclass
class Closure:
    parameter: str
    body: object
    environment: dict


class Machine:
    def __init__(self,fuel=10000):self.fuel=fuel;self.forces=0
    def tick(self):
        self.fuel-=1
        if self.fuel<0:raise RuntimeError('evaluation budget exhausted')
    def evaluate(self,e,env=None):
        self.tick();env={} if env is None else env
        if type(e) in (int,bool):return e
        if e=='nil':return ()
        if isinstance(e,str):return env[e].value
        op=e[0]
        if op=='lam':return Closure(e[1],e[2],dict(env))
        if op in ('let','letrec'):
            local=dict(env);slot=Cell()
            if op=='letrec':local[e[1]]=slot
            slot.value=self.evaluate(e[2],local if op=='letrec' else env)
            local[e[1]]=slot;return self.evaluate(e[3],local)
        if op=='app':
            f=self.evaluate(e[1],env);a=self.evaluate(e[2],env)
            return self.evaluate(f.body,dict(f.environment,**{f.parameter:Cell(a)}))
        if op=='if':return self.evaluate(e[2] if self.evaluate(e[1],env) else e[3],env)
        if op=='case':
            xs=self.evaluate(e[1],env)
            if not xs:return self.evaluate(e[2],env)
            h,t,body=e[3];local=dict(env);local[h]=Cell(xs[0]);local[t]=Cell(xs[1])
            return self.evaluate(body,local)
        a=self.evaluate(e[1],env);b=self.evaluate(e[2],env)
        return {'add':lambda:a+b,'mul':lambda:a*b,'eq':lambda:a==b,'cons':lambda:(a,b)}[op]()


def compile_code(e):
    if type(e) in (int,bool) or e=='nil':return [('CONST',() if e=='nil' else e)]
    if isinstance(e,str):return [('LOAD',e)]
    op=e[0]
    if op=='lam':return [('CLOSURE',e[1],compile_code(e[2]))]
    if op in ('let','letrec'):
        return [(op.upper(),e[1],compile_code(e[2]),compile_code(e[3]))]
    if op=='if':return compile_code(e[1])+[('IF',compile_code(e[2]),compile_code(e[3]))]
    if op=='case':
        h,t,b=e[3]
        return compile_code(e[1])+[('CASE',compile_code(e[2]),h,t,compile_code(b))]
    return compile_code(e[1])+compile_code(e[2])+[(op.upper(),)]


class VM(Machine):
    def execute(self,code,env=None):
        env={} if env is None else env;stack=[]
        for instruction in code:
            self.tick();op=instruction[0]
            if op=='CONST':stack.append(instruction[1])
            elif op=='LOAD':stack.append(env[instruction[1]].value)
            elif op=='CLOSURE':stack.append(Closure(instruction[1],instruction[2],dict(env)))
            elif op in ('LET','LETREC'):
                _,n,value,body=instruction;local=dict(env);slot=Cell()
                if op=='LETREC':local[n]=slot
                slot.value=self.execute(value,local if op=='LETREC' else env)
                local[n]=slot;stack.append(self.execute(body,local))
            elif op=='IF':stack.append(self.execute(instruction[1] if stack.pop() else instruction[2],env))
            elif op=='CASE':
                xs=stack.pop();_,empty,h,t,body=instruction
                if not xs:stack.append(self.execute(empty,env))
                else:
                    local=dict(env);local[h]=Cell(xs[0]);local[t]=Cell(xs[1])
                    stack.append(self.execute(body,local))
            elif op=='APP':
                a=stack.pop();f=stack.pop()
                stack.append(self.execute(f.body,dict(f.environment,**{f.parameter:Cell(a)})))
            else:
                b=stack.pop();a=stack.pop()
                stack.append({'ADD':lambda:a+b,'MUL':lambda:a*b,'EQ':lambda:a==b,
                              'CONS':lambda:(a,b)}[op]())
        if len(stack)!=1:raise ValueError('invalid compiled stack shape')
        return stack[0]


@dataclass
class Thunk:
    expression: object
    environment: dict
    state: str = 'pending'
    value: object = None


class Lazy(Machine):
    def force(self,thunk):
        self.tick()
        if thunk.state=='done':return thunk.value
        if thunk.state=='busy':raise RuntimeError('black hole')
        thunk.state='busy';self.forces+=1
        try:thunk.value=self.evaluate(thunk.expression,thunk.environment)
        except BaseException:
            thunk.state='pending';raise
        thunk.state='done';thunk.expression=None;thunk.environment={}
        return thunk.value
    def evaluate(self,e,env=None):
        self.tick();env={} if env is None else env
        if type(e) in (int,bool):return e
        if e=='nil':return ()
        if isinstance(e,str):return self.force(env[e])
        op=e[0]
        if op=='lam':return Closure(e[1],e[2],dict(env))
        if op in ('let','letrec'):
            local=dict(env);thunk=Thunk(e[2],dict(env));local[e[1]]=thunk
            if op=='letrec':thunk.environment=local
            return self.evaluate(e[3],local)
        if op=='app':
            f=self.evaluate(e[1],env);local=dict(f.environment)
            local[f.parameter]=Thunk(e[2],dict(env));return self.evaluate(f.body,local)
        if op=='if':return self.evaluate(e[2] if self.evaluate(e[1],env) else e[3],env)
        if op=='cons':return Thunk(e[1],dict(env)),Thunk(e[2],dict(env))
        if op=='case':
            xs=self.evaluate(e[1],env)
            if not xs:return self.evaluate(e[2],env)
            h,t,body=e[3];local=dict(env);local[h],local[t]=xs
            return self.evaluate(body,local)
        a=self.evaluate(e[1],env);b=self.evaluate(e[2],env)
        return {'add':lambda:a+b,'mul':lambda:a*b,'eq':lambda:a==b}[op]()


def optimize(e):
    if not isinstance(e,tuple):return e
    op=e[0]
    if op=='lam':return op,e[1],optimize(e[2])
    if op in ('let','letrec'):return op,e[1],optimize(e[2]),optimize(e[3])
    if op=='case':return op,optimize(e[1]),optimize(e[2]),(*e[3][:2],optimize(e[3][2]))
    args=tuple(optimize(c) for c in e[1:])
    if op in ('add','mul','eq') and all(type(c) is int for c in args):
        a,b=args;return {'add':lambda:a+b,'mul':lambda:a*b,'eq':lambda:a==b}[op]()
    if op=='add' and type(args[1]) is int and args[1]==0:return args[0]
    if op=='mul' and type(args[1]) is int and args[1]==1:return args[0]
    if op=='if' and type(args[0]) is bool:return args[1] if args[0] else args[2]
    return (op,*args)


def main():
    p=argparse.ArgumentParser(description='typed functional-language reference machines')
    p.add_argument('source',type=Path);p.add_argument('--mode',choices=['strict','vm','lazy'],default='vm')
    p.add_argument('--fuel',type=int,default=10000)
    args=p.parse_args()
    try:
        e=parse(args.source.read_text());types=Types();types.infer(e)
        e=optimize(e)
        if args.mode=='vm':result=VM(args.fuel).execute(compile_code(e))
        else:result=(Lazy(args.fuel) if args.mode=='lazy' else Machine(args.fuel)).evaluate(e)
        if type(result) not in (int,bool):raise ValueError('CLI prints scalar results only')
        print(result)
    except (ValueError,TypeError,RuntimeError,OSError,RecursionError) as error:p.exit(2,f'{error}\n')


if __name__=='__main__':main()
