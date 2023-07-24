#!/bin/bash
FS_MB=2

######### File system constants ##################
# Sizes for SPIFFS
# # Should be 1028096,  2076672, or  3125248 (1MB, 2MB, or 3MB)
# Sizes for LittleFS
# # Should be 1024000, 2072576, or  3121152 (1MB, 2MB, or 3MB)
# # Should be 0x300000 for 1MB, 0x200000 for 2MB, or 0x100000 for 3MB
# FS_ADDR=0x200000
####################################################
if [ $FS_MB -eq 1 ]; then
    EESZ=4M1M
    FS_SIZE=1024000
    FS_ADDR=0x300000
elif [ $FS_MB -eq 2 ]; then
    EESZ=4M2M
    FS_SIZE=2072576
    FS_ADDR=0x200000
elif [ $FS_MB -eq 3 ]; then
    EESZ=4M3M
    FS_SIZE=3121152
    FS_ADDR=0x100000
else
    echo "Incorrect file system size"
    exit 1
fi

BOARD=esp8266:esp8266:nodemcuv2:xtal=80,vt=flash,exception=disabled,stacksmash=disabled,ssl=all,mmu=3232,non32xfer=fast,eesz=$EESZ,led=2,ip=lm2f,dbg=Disabled,lvl=None____,wipe=none,baud=115200

###########################################################

ARDUINO_CLI=$(find ~/Downloads -type f -name arduino-cli | head -n1)

function compile {
    echo Building sketch
    $ARDUINO_CLI compile $SKETCH_FILE --fqbn $BOARD --build-path $BUILD_DIR
}

function upload {
    echo Uploading sketch
    if [ $IP ]; then
        $ARDUINO_CLI upload --fqbn $BOARD --input-dir $BUILD_DIR --port $IP
    else
        $ARDUINO_CLI upload --fqbn $BOARD --input-dir $BUILD_DIR --port $PORT --verify
    fi
}

function filesystem {
    if [ -e $SKETCH_FOLDER/data/ ]; then
        MKFS=$(find ~/.ardui* -type f -name mklittlefs | head -n1)
        ESPTOOL=$(find ~/.ardui* -type f -name upload.py | head -n1)
        ESPOTA=$(find ~/.ardui* -type f -name espota.py | head -n1)
        
        SOURCE_FOLDER=$(basename $SKETCH_FOLDER)
        DATADIR=$SKETCH_FOLDER/data
        FS_IMG=/tmp/arduino-build-$SKETCH_NAME/$SKETCH_NAME.mklittlefs.bin
        PYTHON=$(find ~/.ardui* -type f -name python3 | head -n1)

        # Clean destination folder
        rm $DATADIR/*.*
        rm -r $DATADIR/*

        # Minify and copy files for device file system
        $(pwd)/minifyFS.sh $SOURCE_FOLDER
        # Create file system image destination folder if required
        mkdir -p /tmp/arduino-build-$SKETCH_NAME

        echo Building file system
        $MKFS -c $DATADIR -p 256 -b 8192 -s $FS_SIZE $FS_IMG
        echo 
        echo Uploading file system
        if [ $IP ]; then
            echo IP
            $PYTHON $ESPOTA -i $IP -s -f $FS_IMG
        else
            echo Serial
            $PYTHON $ESPTOOL --chip esp8266 --port $PORT --baud 115200 write_flash $FS_ADDR $FS_IMG
        fi
    fi
}

if [ "$#" -eq 0 ]; then
    echo Sketch folder is required
    echo -e "Example:\n\tbuild.sh sketch_folder"
    exit 1
else
    SKETCH_FOLDER=$(realpath $1)
    SKETCH_FILE=$(ls -1 $1/*.ino | head -n1)
    SKETCH_NAME=$(basename $1)
    BUILD_DIR=/tmp/arduino-build-$SKETCH_NAME/
    if [ "$#" -eq 1 ]; then
        COMMAND=all
    else
        COMMAND=$2
    fi
    if [ "$#" -eq 3 ]; then
        IP=$3
    else
        PORT=/dev/ttyUSB0
    fi
fi

if [ $COMMAND == "all" ]; then
    compile
    retVal=$?
    if [ $retVal -ne 0 ]; then
        echo Compilation error
        exit 1
    else
        upload
        retVal=$?
        if [ $retVal -ne 0 ]; then
            echo Uploading error
            exit 1
        else
            filesystem
        fi
    fi
elif [ $COMMAND == "compile" ]; then
    compile
elif [ $COMMAND == "flash" ]; then
    compile
    retVal=$?
    if [ $retVal -ne 0 ]; then
        echo Compilation error
        exit 1
    else
        upload
    fi
elif [ $COMMAND == "fs" ]; then
    filesystem
else
    echo Unknown argument $command
fi
