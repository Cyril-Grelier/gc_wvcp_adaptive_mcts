#!/bin/bash

cd docs || exit
make clean
make html
echo "index : docs/_build/html/index.html"
