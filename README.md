# IgnoreACLs
A driver to make windows ignore Access Control when accessing files

Usage:

0. Enable test signing mode on your OC

1. Install using the ini file

2. run: fltmc load IgnoreACLsDriver

3. run: fltmc attach IgnoreACLsDriver c:

4. do whatever you want

5. run: fltmc detach IgnoreACLsDriver c:

6. run: fltmc unload IgnoreACLsDriver