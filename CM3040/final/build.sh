#!/bin/bash
FS_MB=2

######### File system constants ##################
# Values taken from boards.txt in the esp8266 package hardware folder
# AS OF VERSION 2.7.1 OF THE ESP8266 CORE, SPIFFS IS DEPRECATED.
# ANY INFORMATION REGARDING SIZE AND BASE ADDRESS SHOULD BE 
# CONSIDERED UNRELIABLE AND OUTDATED
# Sizes for SPIFFS
# # Should be 1028096,  2076672, or  3125248 (1MB, 2MB, or 3MB)
# Sizes for LittleFS
# # Should be 1024000, 2072576, or  3121152 (1MB, 2MB, or 3MB)
FS_BASE_ADDRESS=0x3FA000 # spiffs_end definition
BLOCK_SIZE=8192 # spiffs_blocksize definition
####################################################
# EESZ definitios also come from boards.txt
if [ $FS_MB -eq 1 ]; then
    EESZ=4M1M
    FS_ADDR=0x300000 # spiffs_start definition
elif [ $FS_MB -eq 2 ]; then
    EESZ=4M2M
    FS_ADDR=0x200000 # spiffs_start definition
elif [ $FS_MB -eq 3 ]; then
    EESZ=4M3M
    FS_ADDR=0x100000 # spiffs_start definition
else
    echo "Incorrect file system size"
    exit 1
fi

FS_SIZE=$((FS_BASE_ADDRESS - FS_ADDR))

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
    DATADIR=$SKETCH_FOLDER/data

    # Ensure the correct versions of the tools are used
    MKFS=$(find ~/.arduino*/packages/esp8266 -type f -name mklittlefs | head -n1)
    ESPTOOL=$(find ~/.arduino*/packages/esp8266 -type f -name upload.py | head -n1)
    ESPOTA=$(find ~/.arduino*/packages/esp8266 -type f -name espota.py | head -n1)
    PYTHON=$(find ~/.arduino*/packages/esp8266 -type f -name python3 | head -n1)
    
    FS_IMG=/tmp/arduino-build-$SKETCH_NAME/$SKETCH_NAME.mklittlefs.bin

    # Minify and copy files for device file system
    $(pwd)/minifyFS.sh fs/$SKETCH_NAME $DATADIR
    # Create file system image destination folder if required
    mkdir -p /tmp/arduino-build-$SKETCH_NAME

    echo Building file system
    $MKFS -c $DATADIR -p 256 -b $BLOCK_SIZE -s $FS_SIZE $FS_IMG
    echo 
    echo Uploading file system
    if [ $IP ]; then
        echo IP
        $PYTHON $ESPOTA -i $IP -s -f $FS_IMG
    else
        echo Serial
        $PYTHON $ESPTOOL --chip esp8266 --port $PORT --baud 115200 write_flash $FS_ADDR $FS_IMG
    fi
    echo \n
}

if [ "$#" -eq 0 ]; then
    echo Sketch folder is required
    echo -e "Example:\n\tbuild.sh path_to_SKETCH_FOLDER [OPERATION] [IP]"
    echo -e "\tOPERATION options:"
    echo -e "\t\tcompile \tCompiles the sketch in the folder"
    echo -e "\t\tflash   \tCompiles and uploads the sketch in the folder. If IP is provided, it tries to perform an OTA upload, otherwise it defaults to /dev/ttyUSB0."
    echo -e "\t\tfs      \tMinifies and tries to upload a filesystem image from fs/SKETCH_FOLDER. If IP is provided, it tries to perform an OTA upload, otherwise it defaults to /dev/ttyUSB0."
    echo -e "\tIf no operation is provided, all of them are performed."
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
