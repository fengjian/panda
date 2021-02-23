import os
from pandare import Panda

arch = 'x86_64'
panda = Panda(generic=arch)

if not os.path.isfile(f'test-{arch}-rr-nondet.log'):
    print("Run gen_rec.py first to generate recording")
else:
    panda.load_plugin("trace", {'log': f'trace_{arch}.txt'})
    panda.run_replay(f"test-{arch}")

    print(f"Trace saved to: trace_{arch}.txt")
