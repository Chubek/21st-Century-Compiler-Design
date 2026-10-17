"""Finite verification and worked-example arithmetic for source chapters 69--75."""
from itertools import product
import unittest
from models import layout, sysv_arguments, encode, decode, Registration, Handles


class Models(unittest.TestCase):
    def test_layout_and_register_rollback(self):
        self.assertEqual(layout([(1,1),(4,4),(8,8)]),([0,4,8],16,8))
        self.assertEqual(layout([(8,8),(4,4)]),([0,8],16,8))
        locations=sysv_arguments(['i64']*6+['pair','f64'])
        self.assertEqual(locations[-2:], [('stack',),((),(0,))])
        self.assertEqual(sysv_arguments(['i64']*6,True)[-1],('stack',))
        self.assertEqual(sysv_arguments(['i64']*8)[-2:],[('stack',),('stack',)])

    def test_wire_roundtrip_and_rejection(self):
        for n in range(65):
            payload=bytes((i*73) % 256 for i in range(n))
            for tag in (0,9,255):
                packet=encode(tag,payload)
                self.assertEqual(decode(packet),(tag,payload))
                for cut in range(len(packet)):
                    with self.assertRaises(ValueError):
                        decode(packet[:cut])
                with self.assertRaises(ValueError):
                    decode(packet+b'!')
        self.assertEqual(encode(9,b'A\0B'),b'\x09\x03\0\0\0A\0B')
        with self.assertRaises(ValueError):
            decode(b'\x09\xff\xff\xff\xff')

    def test_every_short_registration_trace(self):
        for events in product(('enter','leave','stop','collect'),repeat=7):
            state=Registration()
            active=0
            for event in events:
                next_state=state.step(event)
                if next_state is None:
                    break
                if event == 'enter': active += 1
                if event == 'leave': active -= 1
                self.assertEqual(next_state.active,active)
                self.assertFalse(next_state.freed and (active or next_state.accepting))
                if state.freed:
                    self.assertTrue(next_state.freed)
                state=next_state
        s=Registration().step('enter').step('stop')
        self.assertIsNone(s.step('collect'))
        self.assertTrue(s.step('leave').step('collect').freed)

    def test_handles_survive_moves_and_reject_stale(self):
        h=Handles(); a=h.create(41); b=h.create(42)
        old=h.get(a)
        for _ in range(20):
            h.move()
            self.assertEqual((h.get(a)['value'],h.get(b)['value']),(41,42))
        self.assertIsNot(h.get(a),old)
        h.release(a)
        c=h.create(99)
        self.assertEqual(a[0],c[0]); self.assertNotEqual(a[1],c[1])
        with self.assertRaises(ValueError): h.get(a)
        with self.assertRaises(ValueError): h.release(a)
        self.assertEqual(h.get(c)['value'],99)


if __name__ == '__main__':
    unittest.main(verbosity=2)
