"""Chapters 69--75: bounded protocol and target-layout models, not native ABIs."""
from dataclasses import dataclass


def layout(fields):
    """Ordinary nonpacked fields with power-of-two alignments; no bit-fields."""
    offset, maximum, offsets = 0, 1, []
    for size, alignment in fields:
        if size < 0 or alignment <= 0 or alignment & (alignment-1):
            raise ValueError('unsupported layout')
        offset += (-offset) % alignment
        offsets.append(offset)
        offset += size
        maximum = max(maximum, alignment)
    return offsets, offset + (-offset) % maximum, maximum


def sysv_arguments(types, hidden_result=False):
    """Restricted SysV AMD64 model: i64, f64, pair(double,int32), triple(u64*3).
    Only class/register decisions are modeled; stack byte placement is not.
    """
    gp, fp, result = int(hidden_result), 0, []
    for kind in types:
        need_gp, need_fp = {'i64': (1,0), 'f64': (0,1), 'pair': (1,1),
                            'triple': (7,0)}[kind]
        if gp+need_gp > 6 or fp+need_fp > 8:
            result.append(('stack',))
        else:
            result.append((tuple(range(gp,gp+need_gp)), tuple(range(fp,fp+need_fp))))
            gp += need_gp
            fp += need_fp
    return result


def encode(tag, payload):
    if not 0 <= tag <= 255 or len(payload) > 1048576:
        raise ValueError('wire range')
    return bytes([tag]) + len(payload).to_bytes(4,'little') + payload


def decode(packet):
    if len(packet) < 5:
        raise ValueError('short header')
    length = int.from_bytes(packet[1:5],'little')
    if length > 1048576 or length != len(packet)-5:
        raise ValueError('invalid payload extent')
    return packet[0], packet[5:]


@dataclass(frozen=True)
class Registration:
    """Atomic owner transitions; collected storage must never have active users."""
    accepting: bool = True
    active: int = 0
    freed: bool = False

    def step(self, event):
        if event == 'enter':
            if not self.accepting or self.freed:
                return None
            return Registration(True,self.active+1,False)
        if event == 'leave':
            if not self.active or self.freed:
                return None
            return Registration(self.accepting,self.active-1,False)
        if event == 'stop':
            return Registration(False,self.active,self.freed)
        if event == 'collect':
            if self.accepting or self.active:
                return None
            return Registration(False,0,True)
        raise ValueError(event)


class Handles:
    """Sequential moving-heap model; generations distinguish reused slots."""
    def __init__(self):
        self.slots = []

    def create(self, value):
        for index, (generation, obj) in enumerate(self.slots):
            if obj is None:
                self.slots[index] = generation, {'value':value}
                return index,generation
        self.slots.append((0,{'value':value}))
        return len(self.slots)-1,0

    def get(self, handle):
        index,generation = handle
        if not 0 <= index < len(self.slots):
            raise ValueError('invalid handle')
        current,obj = self.slots[index]
        if generation != current or obj is None:
            raise ValueError('stale handle')
        return obj

    def move(self):
        self.slots = [(g,dict(o) if o is not None else None) for g,o in self.slots]

    def release(self, handle):
        self.get(handle)
        index,generation = handle
        self.slots[index] = generation+1,None
