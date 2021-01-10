![GitHub top language](https://img.shields.io/github/languages/top/fytex/Shafa?style=for-the-badge)


# What is Shafa?

Shafa is a free and open-source program made for Lossless files' compression written in C 2020/21.  
It is OS independent and some modules are multithread executed.  
Uses two algorithms RLE and Shannon Fano with blocks of length 1 (K=1).  
For RLE decompression a .freq file is needed along with the .rle file.  
For Shannon-Fano decompression a .cod file is needed along with .shaf.  

## Modules:
  - f ( RLE compression and Frequencies calculation )
  - t ( Codes calculation using Shannon-Fano's algorithm )
  - c ( Shannon-Fano compression )
  - d ( RLE and Shannon-Fano decompression )

### Block Size:
  - K = 640 KiB
  - m =   8 MiB
  - M =  64 MiB

### CLI Options:
    -m <module>      :  Executes respective module (Can be executed more than one module if possible)
    -b <K/m/M>       :  Block size for compression (default: K)
    -c <r/f>         :  Forces execution (r -> RLE's compress | f -> Original file's frequencies)
    -d <s/r>         :  Only executes a specific decompression (s -> Shannon-Fano's algorithm | r -> RLE's algorithm)
    --no-multithread :  Disables multithread 

**Note:** Multithread was only implemented in modules C and D (ones that cost the most)

### SETUP - \*NIX
```
gcc -o shafa $(find ./src -name '*.c' -or -name '*.h') -O3 -Wno-format -pthread
```

### SETUP - WINDOWS
```
gcc -o shafa src/*.c src/*.h src/*/*.c src/*/*.h src/*/*/*.c src/*/*/*.h -O3 -Wno-format
```
