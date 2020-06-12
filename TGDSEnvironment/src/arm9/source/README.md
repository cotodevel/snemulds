ToolchainGenericDS-Filesystem driver (a.k.a TGDS FS) adds the following features into ToolchainGenericDS:
   - FatFS FAT16/FAT32/exFAT disk library for the Nintendo DS
   - Enables POSIX -> Newlib File IO Operations such as fopen/fread/fwrite/fclose/fseek/fgetc/etc
   - FileClass layer Implementation: Simulates libfat in the high level layer (about 99%) so it aids in porting legacy NDS Homebrew.
   - TGDS File / Directory Iterator Operator: Instances (Keeping memory footpring low) / Push-Pop operations, preventing to rewrite a lot of useless C code.

Coto
