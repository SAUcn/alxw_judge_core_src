#!/usr/bin/env python
import re

with open('/usr/include/asm/unistd_64.h') as f:
    defs = re.findall(r'#define __NR_(\w+)\s+(\d+)', f.read())

for name, num in defs:
    print(name, num)
