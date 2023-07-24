# Delivered implementation

## Folder structure

* `docs` - Project report and documentation as delivered.
* `firmware` - Source codes for the main and sensor devices.
* `fs` - File system sources for the main and sensor devices.
* `hw` - Fritzing diagrams of the turned in devices.
* `build.sh` - Bash script for compiling the code and file system for the main device and sensor node(s).
* `notes.txt` - Some notes from the development phase regarding bugs and prospective changes.

## Requirements and dependencies

### Node.js dependencies for compilation

```
$ npm install -g html-minifier
$ npm install -g cssnano-cli
$ npm install -g uglify-js
```

### Required arduino libraries

See `main.ino` and `sense.ino` in the firmware folder.
