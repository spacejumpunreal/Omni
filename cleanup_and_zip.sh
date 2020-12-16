#!/bin/sh
git clean -xdf;
git archive -o Omni.zip --format zip HEAD;
echo "hello"
