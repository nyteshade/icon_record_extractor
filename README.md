# icon_record_extractor
Extracts Icns Records from macOS icon files

## Overview
This is a simple C application that extracts all the individual records from a given .icns macOS icon file. If the record is an icon record (loosely as not all cases are handled) then the file extracted will be given its own magic keyword and made into an icon file containing only that record.

## Cases Handled
The logic is simple and there is almost no error handling in this release. The identifier OSType is the filename plus .icns except when that identifier is `info` in which case .plist will be added and the filename will be info.plist. Additionally the special identifier indicating a dark mode icon will create a file called `dark.icns`. This contents of this record is a whole icon file in itself minus the magic keyword and filesize. Those are added and the dark.icns file is extracted.

 * `FD D9 2F A8` dark mode icns file extraction
 * `info` identifier icons are extracted as info.plist files
 * All other identifiers, correctly or incorrectly, are extracted as .icns files.
 
## Missing?
Any error handling.

## Instructions
```sh
Usage: icon_records <file.icns> [<identifier> [<outputPathAndName>]]
Where
  <file.icns>          - replace this with the name of the icns file to extract from
  <identifier>         - optionally only extract a single record's data. Supply the
                         four letter identifier for the record. Use "dark" for darkmode
                         icons. These are usually only Apple system folder icons.
  <outputPathAndName>  - an optional name other than "<identifier>.icns". Only used when
                         desired record exsits and is found and is extracted.
                         
 RC is 0 for success
 RC is 1 for something went wrong
```

## Examples

```sh
$ icon_records GenericFolder.icns
```
Will extract all records in the GenericFolder.icns in the current working directory

```sh
$ icon_records GenericFolder.icns ic10
```
Will extract the 1024x1024 ic10 file as `icn10.icns` in the current working directory

```sh
$ icon_records GenericFolder.icns dark GenericFolderDark.icns
```
Will extract the `FD D9 2F A8` special dark mode icon from the .icns file if there is one and provide the magic number and the associated size to allow it to function as an .icns file. Instead of dark.icns, this invocation will create GenericFolderDark.icns as the output.

## Credits
Almost all impetus for this was gleaned from avl7771's archive https://github.com/avl7771/createicns
