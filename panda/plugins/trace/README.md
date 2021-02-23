Plugin: Trace
===========

Summary
-------
Dump all memory and register changes to disk. This is massive and slow.
You want to do this on a replay.

Arguments
---------
log: file name to store results in. Defualts to `trace.txt`

Dependencies
------------

APIs and Callbacks
------------------

Log format
-----
Line starts with `[current asid],[kernel_mode]` then a list of register value changes, memory reads, and memory writes.

```
0x1234,rax=0x4,rbx=0x1e288060,rcx=0x1e288060,rdx=0x403b870,rbp=0x1,rsp=0xbae438,rsi=0x2e1c0030,rdi=0x403b840,r8=0x0,r9=0x230,r10=0x403b870,r11=0x0,r12=0xffffffff,r13=0x2c7cb684b38,r14=0x141281d08,r15=0x1,rip=0x1401f6530,
0x1234,rip=0x1401f6535,mw=0xbae450:0x6080281E00000000,
rip=0x1401f653a,mw=0xbae458:0x0100000000000000,
rsp=0xbae430,rip=0x1401f653b,mw=0xbae430:0x40B8030400000000,
rsp=0xbae000,rip=0x1401f6542,
rdi=0x403b870,rip=0x1401f6545,
rip=0x1401f6549,mw=0x1e288062:0x00,
```

mw = memory write, mr = memory read. Lines have a trailing newline. Blank lines happen sometimes.


Example
-------

```
-replay my_replay -panda trace:log=log.txt
```
