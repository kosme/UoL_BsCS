#!/bin/bash

function minify {
    echo 
    echo Minifying $INPUT_FOLDER into $OUTPUT_FOLDER

    # Create output folder if necessary
    if ! [ -e $OUTPUT_FOLDER ]; then
        mkdir $OUTPUT_FOLDER
    else
        # Clean destination folder if it exists
        rm $OUTPUT_FOLDER/*.*
        rm -r $OUTPUT_FOLDER/*
    fi

    for file in $(ls -1 $INPUT_FOLDER/*.html)
    do
        html-minifier --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes --remove-tag-whitespace --use-short-doctype $file -o firmware/$folder/data/$(basename $file)
        echo "$(basename $file) minified"
    done

    for file in $(ls -1 $INPUT_FOLDER/*.css)
    do
        cssnano $file $OUTPUT_FOLDER/$(basename $file)
        echo "$(basename $file) minified"
    done

    for file in $(ls -1 $INPUT_FOLDER/*.js)
    do
        uglifyjs --compress --mangle toplevel -o $OUTPUT_FOLDER/$(basename $file) -- $file
        echo "$(basename $file) minified"
    done

    for file in $(ls -1 $INPUT_FOLDER/*.json)
    do
        $(python3 -c 'import json, sys;json.dump(json.load(sys.stdin), sys.stdout)'  < $file >  $OUTPUT_FOLDER/$(basename $file))
        echo "$(basename $file) minified"
    done

    cp -r $INPUT_FOLDER/img $OUTPUT_FOLDER/.
    echo "Images copied"
}

if [ "$#" -le 1 ]; then
    # Hardcoded values for independent use on this project
    for folder in main sense
    do
        INPUT_FOLDER=fs/$folder
        OUTPUT_FOLDER=firmware/$folder/data
        minify
    done
else
    # Prepare global definitions
    INPUT_FOLDER=$(realpath $1)
    OUTPUT_FOLDER=$(realpath $2)
    minify
fi
