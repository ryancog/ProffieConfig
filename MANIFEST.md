# Manifest Formatting

The manifest is a pconf file, with the following sections

MESSAGE includes relevant version label (launcher version) w/ <=>
- May contain "FATAL" marker to indicate the launcher can no longer be used for updating.
- CONTENT includes message to display

Every included individual item is listed:
- EXEC: Executable Binary
- LIB: Base Library
- COMP: Component Library
- RSRC: Resource File

Each individual item specifies path relative to base for respective item type.
- PATH_macOS, PATH_Linux, and PATH_Win32
- HIDDEN may be flagged to hide the item in changelog
Versions are listed with hash (HASH_macOS, HASH_Linux, and HASH_Win32) for each platform.
Each version may also contain:
- FIX: entries w/ bugfixes
- CHANGE: entries w/ behavior changes/updates
- FEAT: entries w/ new features added

BUNDLE lists all files which make up a complete version bundle
- Files listed in the form: TYPE("NAME"): "VERSION"
- NOTE entry may be added for general release messages
