# strings2
Strings2 is a Windows 32bit and 64bit command-line tool for extracting strings from binary data. On top of the classical Sysinternals strings approach, this improved version is also able to dump strings from process address spaces and also reconstructs hidden assembly local variable assignment ascii/unicode strings. Currently, the ASM-string extracting approach only supports the x86 instruction set.

I am maintaining a public binary release download page for this project at:
  http://split-code.com/strings2.html


## Flags
The command-line flags for strings2 are as follows:

	 -f
	Prints the filename/processname before each string.
	
	 -r
	Recursively process subdirectories.
	
	 -t
	Prints the type before each string. Unicode,
	ascii, or assembly unicode/ascii stack push.
	
	 -asm
	Only prints the extracted ascii/unicode
	assembly stack push-hidden strings.
	
	 -raw
	Only prints the regular ascii/unicode strings.
	
	 -l [numchars]
	Minimum number of characters that is
	a valid string. Default is 4.
	
	 -nh
	No header is printed in the output.
	
	 -pid
	The strings from the process address space for the
	specified PID will be dumped. Use a '0x' prefix to
	specify a hex PID.
	 
	 -system
	Dumps strings from all accessible processes on the
	system. This takes awhile.

		
## Example Usage
From the command prompt:
* strings2 malware.exe
* strings2 *.exe > strings.txt
* strings2 *.exe -nh -f -t -asm > strings.txt
* strings2 -pid 419 > process_strings.txt
* strings2 -pid 0x1a3 > process_strings.txt
* strings2 -system > all_process_strings.txt
* cat abcd.exe | strings2 > out.txt


## Contributing
Contributions are welcome. Some possible contribution directions are as follows:
* Only print unique strings.
* Add flag support for dumping process strings by process/window title matching.
* Add x64 assembly support for extracting ASM stack pushed strings.
