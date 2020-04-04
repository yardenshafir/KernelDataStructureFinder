# KernelDataStructureFinder - WinDBG scripts

## Warning - Very slow!!

This directory contains ugly but working WinDBG scripts to dump data about all lookaside lists / resources in the kernel.
There are separate scripts to dump information about structures which are in the pool and ones inside of drivers.
Resources only have one script, to dump information about resources in the pool, 
but the script "FindSymbolForLookasideList.txt" can be modified to dump resources by replacing the symbol nt!ExPagedLookasideListHead with nt!ExpSystemResourcesList.

The lookaside list scripts iterate over paged lookasides, but in order to dump information about all different lookaside lists, the scripts should be edited to iterate over all of these lists:
1. nt!ExPagedLookasideListHead (default)
2. nt!ExNPagedLookasideListHead
3. nt!ExPoolLookasideListHead
4. nt!ExSystemLookasideListHead

An interesting use of these scripts is finding lookaside lists or resources embedded inside larger structures.
In order to only show structures whose size is larger than the lookaside / resource, add the following command:
* For resources: 
    dx -r1 @$poolData.Where(l => l.size != "0x80")
* For lookaside lists:
    dx -r1 @$poolData.Where(l => l.size != "0x110")

To run the scripts copy the commands from the file into WinDbg and execute.
