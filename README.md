# A Multithreading-based Approach to Optimizing HashLife on a Multi-Core CPU
## Code Repository for my Extended Essay

In this repository, you will find: 

+ `hashlife.cpp`: My default implementation of the HashLife algorithm without multithreading
+ `threadlife.cpp`: My implementation of HashLife with multithreading 
+ `rle_parser.py`: A GOL pattern file parser which turns a pattern file from an encoded format (RLE) to a loadable configuration into the hashlife programs
+ `patterns/`: A directory containing the siz pattern files used in my Extended Essay.

## Pattern Files

Each pattern file is stored in RLE (Run-length Encoded) format. For more information on this particular file format, see 

https://conwaylife.com/wiki/Run_Length_Encoded

Here is information about the pattern files used in the experiment:

| Pattern # | Adjusted Pattern Size (N * N) | File Name |
| --------- | -------------------- | --------- |
| 1 | 32 | `popover.rle` |
| 2 | 64 | `112p51extended.rle` |
| 3 | 128 | `tubwithtal_synth.rle` |
| 4 | 256 | `112p51_synth.rle` |
| 5 | 512 | `109-still-lifes.rle` |
| 6 | 1024 | `prime_calculator.rle` |

To view these pattern files, you can use the online GOL simulator and pattern viewer http://copy.sh/life/. To do this, download the desired pattern file, then on the website press "Import" and select the downloaded file. 