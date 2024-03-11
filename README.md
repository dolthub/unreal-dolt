# Dolt For Unreal

This is a plugin for Unreal Editor that adds Editor Actions (context-sensitive actions when right-clicking on Assets in the Content Browser) and Console Commands that allow for interoperability between Unreal Editor's Data Tables and [Dolt](https://dolthub.com)

You can read our announcement post [here](https://dolthub.com/blog/2024-03-11-dolt-plus-unreal/).

## Disclaimer

This is a proof of concept. Play around with it, file issues, but don't use it for critical data.

## Installation

Simple download the source code and copy the `Plugins` directory into your Unreal project. The next time you open the project in the editor, you will be asked if you want to compile the plugin.

Most of our testing was performed on Unreal 5.3, on MacOS and Linux, with Perforce as the main VCS. But in theory this plugin should work for other versions of Unreal, and with every OS and VCS that Unreal Editor supports. If you encounter problems with your configuration, please file an [issue](https://github.com/dolthub/unreal-dolt/issues).

## How to Use

After installing, configure the plugin via `Edit > Project Settings`

Right clicking on any Data Table in the Content Browser will provide the following context-sensitive options in the `Scripted Asset Actions > Dolt` submenu:
- Export to Dolt: Copies the table into Dolt
- Import from Dolt: Updates the table in Unreal to match the corresponding table in Dolt.
- Pull Rebase: Sync the table against the most recent source control revision, using Dolt to merge local changes with remote changes.
- Three Way Export: Creates three Dolt commits: a commit for local changes, a commit for remote changes, and a ancestor commit containing the currently synced-to revision of the selected table.
- Force Sync and Import From Dolt: Sync the table against the most recent source control revision, then update the Data Table from Dolt.

Additionally, dolt commands can be run from the Console.

The main use case is resolving concurrent changes to the same Data Table. If the changes do not create a conflict, then `Pull Rebase` will seamlessly combine then. If there is a conflict, use `Three Way Export` to export the table into Dolt, then use `dolt conflicts` in the Console to resolve the conflicts, then run `Force Sync and Import From Dolt`.

## Testing

The provided Dockerfile can be used to run conformance tests in a sandboxed environment. The intent is to allow testing with any combination of:
- Unreal Engine version
- Version Control System
- Operating System

To run the tests, simply run `docker compose run unreal` from the top level of this repo, then look for the string "LogTemp: Display: All tests passed" to be printed to standard output.

Currently, the Docker testing only tests the specific combination "UE5.3-git-Linux". However, this plugin has been manually tested with Perforce and Mac (Silicon).

