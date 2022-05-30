# strings2 - Extract strings from binary files and process memory
Strings2 is a Windows command-line tool for extracting strings from binary data. On top of the classic Sysinternals strings approach, this tool includes:
* Multi-lingual string extraction, such as Russian, Chinese, etc.
* Machine learning model filters out junk erroneous string extractions to reduce noise.
* String extractions from process memory.
* Recursive and wildcard filename matching.
* Json output option for automation integration. (Also see python module version [binary2strings](https://github.com/glmcdona/binary2strings))

I also recommend looking at [FLOSS](https://github.com/mandiant/flare-floss) from Mandiant a cross-platform string extraction solver with a different set of features.

## Installation
Download the [latest release binary](https://github.com/glmcdona/strings2/releases).

## Example Usage

Dump all strings from `malware.exe` to stdout:

* ```strings2 malware.exe```

Dump all strings from all `.exe` files in the `files` folder to the file `strings.txt`:
* ```strings2 ./files/*.exe > strings.txt```

Dump strings from a specific process id, including logging the module name and memory addresses of each match:
* ```strings2 -f -s -pid 0x1a3 > process_strings.txt```

Extract strings from `malware.exe` to a json file:
* ```strings2 malware.exe -json > strings.json```

## Documentation

```strings.exe (options) file_pattern```

* `file_pattern` can be a folder or file. Wildcards (`*`) are supported in the filename parts - eg `.\files\*.exe`.

|Option|Description|
|--|--|
|-r|Recursively process subdirectories.|
|-f|Prints the filename/processname for each string.|
|-F|Prints the full path and filename for each string.|
|-s|Prints the file offset or memory address span of each string.|
|-t|Prints the string type for each string. UTF8, or WIDE_STRING.|
|-wide|Prints only WIDE_STRING strings that are encoded as two bytes per character.|
|-utf|Prints only UTF8 encoded strings.|
|-a|Prints both interesting and not interesting strings. Default only prints interesting non-junk strings.|
|-ni|Prints only not interesting strings. Default only prints interesting non-junk strings.|
|-e|Escape new line characters.|
|-l [num_chars]|Minimum number of characters that is a valid string. Default is 4.|
|-b [start]\(:[end]\)|Scan only the specified byte range for strings. Optionally specify an end offset as well.|
|-pid [pid]|The strings from the process address space for the specified PID will be dumped. Use a '0x' prefix to specify a hex PID.|
|-system|Dumps strings from all accessible processes on the system. This takes awhile.|
|-json|Writes output as json. Many flags are ignored in this mode.|


## Version History

Version 2.0 (May 29, 2022)
  - Complete overhaul of the tool.
  -	Upgrade string extraction engine from [binary2strings](https://github.com/glmcdona/binary2strings).
  - Add support for multilingual strings.
  - Added ML model to filter junk erroneous string extractions.
  - Add option to dump only a specified offset range.
  - Add json output option.
  - Add memory address and module name logging.
  - Fixes to 64bit process string dumping.

Version 1.2 (Apr 21, 2013)
  -   Added "-a" and "-u" flags to extract only ascii or unicode strings.
  -   Fixed a bug when processing certain filenames.

Version 1.1 (Nov 22, 2012)
  -   Added "-r" recursive flag option.
  -   Added "-pid" and "-system" flag options to specify process input sources.
  -   Piped input data is now supported.
  -   Various fixes.

Version 1.0 (Sept 20, 2012)
  -   Initial release.