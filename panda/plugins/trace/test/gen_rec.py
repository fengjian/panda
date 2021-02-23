import os
from pandare import Panda

arch = 'x86_64'
panda = Panda(generic=arch)

@panda.queue_blocking
def driver():
    panda.record_cmd("whoami", recording_name=f"test-{arch}")
    panda.end_analysis()

panda.run()
