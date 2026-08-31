#!/bin/bash

function minify {
    echo 
    echo $folder
    for file in $(ls -1 fs/$folder/*.html)
    do
        html-minifier --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes --remove-tag-whitespace --use-short-doctype $file -o firmware/$folder/data/$(basename $file)
        echo "$(basename $file) minified"
    done

    for file in $(ls -1 fs/$folder/*.css)
    do
        cssnano $file firmware/$folder/data/$(basename $file)
        echo "$(basename $file) minified"
    done

    for file in $(ls -1 fs/$folder/*.js)
    do
        uglifyjs --compress --mangle toplevel -o firmware/$folder/data/$(basename $file) -- $file
        echo "$(basename $file) minified"
    done

    for file in $(ls -1 fs/$folder/*.json)
    do
        $(python3 -c 'import json, sys;json.dump(json.load(sys.stdin), sys.stdout)'  < $file >  firmware/$folder/data/$(basename $file))
        echo "$(basename $file) minified"
    done

    cp -r fs/$folder/img firmware/$folder/data/.
    echo "Images copied"
}

if [ "$#" -eq 0 ]; then
    for folder in main sense
    do
        minify
    done
else
    folder=$1
    minify
fi
