# KernelDataStructureFinder

Writeup: https://windows-internals.com/lookaside-list-forensics/

This driver iterates over all paged lookaside lists, nonpaged lookaside lists and resources
and prints for each the address and whether it is located in a driver or in the pool.
If structure is in a driver, driver name will be printed. If structure is in the pool, 
pool tag will be printed for lookasides but not for resources, since tag is not a part of the ERESOURCE structure and difficult to 
retrieve in Windows10 RS5+.
Before RS5 pool parsing is possible to get tag and size information about resources and lookasides.
