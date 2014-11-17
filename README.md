CoktelADL2VGM README
====================

CoktelADL2VGM is a small tool to convert AdLib music used in games by
[Coktel Vision](https://en.wikipedia.org/wiki/Coktel_Vision) (ADL and MDY+TBR)
into [VGM](http://www.smspower.org/Music/VGMFileFormat). Software to play back
and manipulate VGM files further can be found [here on the SMS Power!
website](http://www.smspower.org/Music/Software).

CoktelADL2VGM is licensed under the terms of the [GNU Affero General Public
License version 3](https://www.gnu.org/licenses/agpl-3.0.html) (or later).

Usage
-----

CoktelADL2VGM is a command line tool and needs to be invoked on the command
line / shell. The list of valid parameters and operational modes is as follows:

    CoktelADL2VGM - Tool to convert Coktel Vision's AdLib music to VGM
    Usage: cokteladl2vgm [options] <file.adl>
           cokteladl2vgm [options] <file.mdy> <file.tbr>
           cokteladl2vgm [options] </path/to/coktel/game/>
    
      -h      --help              Display this text and exit.
      -v      --version           Display version information and exit.

Examples:
- cokteladl2vgm intro.adl  
  Convert the ADL music file intro.adl into the VGM file intro.adl.vgm
- cokteladl2vgm intro.mdy intro.tbr  
  Convert the MDY music file intro.mdy with TBR instrument data intro.tbr
  into the VGM file intro.mdy.vgm
- cokteladl2vgm /games/coktel/gobliiins/  
  Search through all resource files of the Coktel Vision game found
  in /games/coktel/gobliiins/ and convert all ADL and MDY/TBR files
  used by the game into the VGM format
- cokteladl2vgm.exe C:\games\coktel\gobliiins\  
  Like above, but on Windows

All new files will be created in the current working directory.
