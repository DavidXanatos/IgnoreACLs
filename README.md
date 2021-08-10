# IgnoreACLs
A driver to make windows ignore Access Control when accessing files

Usage:

0. Enable test signing mode on your PC

1. Install using the ini file

2. run: "fltmc load IgnoreACLsDriver"

3. run: "fltmc attach IgnoreACLsDriver c:" or replace c: with the drive you want to disable ACLs for

4. now SCL's are disabled so you can do at the drive  whatever you want, i.e. access protected files and fodlers, modify critical system files, etc... once you are done continue to re enabel ACL's

5. run: "fltmc detach IgnoreACLsDriver c:" replace c: with the drive you used in step 3.

6. run: "fltmc unload IgnoreACLsDriver"
