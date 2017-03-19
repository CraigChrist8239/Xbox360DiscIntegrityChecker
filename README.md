# Xbox360DiscIntegrityChecker
Used to verify that Xbox 360 discs can be read and fully played. 

For unmodded dev kits ONLY!! Retail systems obviously cannot run this unsigned code, and JTAGs/RGHs and dev kits with modded recoveries remove disc integrity checking functions. 

Made to verify the integrity of discs for an unnamed business. Small volume so it could be done on my personal dev. 

TO USE:
Compile and copy to your dev. Run the default.xex once to allow for retail discs and exit. Put the disc to check in the system and run default.xex again. It will take time to check the entire disc (optionally use watson to follow progress). The docs say that a single hash error will result in a disc read error, while the docs recommend that if the number of failed reads exceeds 10% of the total blocks, the disc should be considered bad.
