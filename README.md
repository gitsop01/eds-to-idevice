eds-to-idevice
==============
Evolution data server to idevice

This code gas been cloned from https://gitlab.com/eds-to-idevice/eds-to-idevice/tree/master

The original Author of the code is Christophe Fergeau <cfergeau@gmail.com>

Used to sync contact info between Evolution data server and an idevice.

This code has been updating to  EDS version 3.24.3 and compiled with the latest
libimobiledevice library 1.20, using GCC 10.0.0.1

This code has successfully transferred 440 EDS contacts to an idevice
and should compile on most Linux distributions but must be running 
Evolution Data Server as the Source of the synced contacts.
One way sync only.

This is a command line utility - start in a terminal as ./eds-to-idevice
Ensure your phone is plugged in or use the -uuid or -u (40 digit uuid command line argument).
Use -help or -h from the command line for all commandline arguments.
Contact transfer info is sent to the terminal as the program is run.

This utility has been tested on iphone 3, 4 and 7. 

