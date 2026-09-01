# Delivered implementation

## Folder structure

* `docs` - Project report and documentation as delivered.
* `firmware` - Source codes for the main and sensor devices.
* `fs` - File system sources for the main and sensor devices.
* `hw` - Fritzing diagrams of the turned in devices.
* `build.sh` - Bash script for compiling the code and file system for the main device and sensor node(s).
* `minifyFS.sh` - Bash script for minifying the file system for the main device and sensor node(s).
* `notes.txt` - Some notes from the development phase regarding bugs and prospective changes.

## Requirements and dependencies

### External dependencies for using `build.sh`

The build script depends on the Arduino CLI executable (`arduino-cli`). It is assumed that is located in the Downloads folder. Modify line 33 if required.

### Node.js dependencies for minification

```
$ npm install -g html-minifier-next
$ npm install -g cssnano-cli
$ npm install -g uglify-js
```

### Required Arduino libraries

See `main.ino` and `sense.ino` in the firmware folder.
