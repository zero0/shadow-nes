# shadow-nes
## About
A NES game programmed from the ground up using 6502 Assembly and C (using `cc65`) along with a suite of tools programmed in C#.

### shadow.nes
The games ROM playable in a NES emulator.

### img2chr
C# command line tool to process images (now text files and other assets) into a format that can used on the NES.

## Requirements
- VSCode
  - Alchemy65
  - C# Dev Kit
- cc65
  - Currently hardcoded to `C:\cc65\`
- Mesen (not required, but it works well)
- .NET SDK
- Apache Ant (for building)

### Supported Platforms
- Windows
- macOS

## Build
- Run `ant build-nes` or in VSCode `Ctrl+Shift+B`
- Open `bin/shadow.{language}.nes` in Mesen
