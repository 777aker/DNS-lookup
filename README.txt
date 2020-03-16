multi-lookup
----------------------------------
works on Windows and Linux
NOTE:
You are supposed to be able to iterate over the names files
when they are entered like ["names1.txt", "names2.txt"] however
my arguments were picking up the [ and ,'s and I'm pretty sure
that's a terminal issue not something wrong with my program.
argv[5] above would be [names.txt, which seems wrong.
So instead you just do names1.txt names2.txt.
----------------------------------
building
First place your input files, Makefile, multi-lookup.c, multi-lookup.h,
util.c, and util.h in the same folder
Open a terminal in that folder, then type make and the Makefile will
compile everything with -Wall and -Wextra enabled
----------------------------------
running
In order to run, after building open your terminal
in the same folder you built in
For windows type "multi-lookup <# of requesters desired, below 11> <# of resolvers, below 11>
<service file path> <results file path> <first file you want to service> <next file> ... <last
file, no more than 10>
For linux just change multi-lookup to ./multi-lookup
For example to run in the pa3 folder given on Windows
multi-lookup 3 3 serviced.txt results.txt ./input/names1.txt ./input/names2.txt
./input/names3.txt ./input/names4.txt ./input/names5.txt