## PoorChoice

To request an input within batch console (for Windows) in a variable

### Commands

 * **-h**: Help
 * **-l**: Max answer len, between 1 and 99 (default 80)
 * **-r**: Max retry, between 1 and 9 (default 3)
 * **-s**: String to show asking to retry (default "Retry")
 * **-a**: String to show telling that the operation aborted (default "Abort")
 * **-e**: Name of the file that will be created containing the variable, (default "CHOICE")
 * **-x**: Character to use as an abort sign (default '$')

### Example

Batch to load the file into a variable

	@echo off
	cls
	PoorChoice -e TOTAL_CHOICE
	setlocal enabledelayedexpansion
	set CHOICE_VAR=
	for /f "delims=" %%a in (TOTAL_CHOICE) do (
	  set currentline=%%a
	  set CHOICE_VAR=!%CHOICE_VAR%!currentline!
	)

After this you can show the content of the variable

	echo %CHOICE_VAR%

### Caveats

 * If the file is large, your environment will run out of space and you will not be able to place the file's entire contents into your variable.
 * The above loop will skip any blank lines.
 * The filecontents variable will still be just a single line, as variables can't hold multiple lines.
 
## TotalSync

Compares two directories and synchronize the relative content.
Symlinks and Reparse points are avoided.

### Example

	totalSynch -f (master dir) -t (slave dir) (-o[parameters]) [-l secondsLeak] [+... -p (file pattern)]
	
For example to clone a directory, including deletions

	totalSync -f C:\Source -t D:\Dest -orcw

### Parameters

 * **-f**: The first directory involved (in case of unidirectional sync, this will be the source directory, must follow a directory
 * **-t**: The second directory involved (in case of unidirectional sync, this will be the destination directory, must follow a directory
 * **-l**: Maximum difference in seconds allowed for two files to be considered modified in the same time (default to 10 seconds)
 * **-p**: The file pattern to blacklist, with the standard * and ? syntax (* for any number of any character,? for any single character)
 * **-h**: Prints this help
 * **-o**: Options, may be a combination of Simple and Composed suboptions

### Simple options
 * **v**: Verify only modification date and size to check the differences between files
 * **a**: Analyze only, don't make any real modifications but show differences and modifications that will be made
 * **u**: When two files have different dates use the most recent one ignoring the definition of master or slave directory, inhibits the [w] composed sub option;

### Composed sub options:
These sub options can be further enriched with the definition of the direction of the action everyone of them can be written alone or followed by [f] or [t] or [b] that means use the master, the slave or both the directories
 * **c**: Copy the new files from the master (default) or following the direction specifications before. Inhibits the [r] option if in contrast (e.g. a [cs] command will inhibit a [rs] command)
 * **r**: Remove the new files from the slave (default)  or following the direction specifications before. Inhibits the [c] option if in contrast
 * **w**: Consider the master as the sole source (default), inhibits the [u] sub option, can't accept the [b] direction, only [f] or [t]
