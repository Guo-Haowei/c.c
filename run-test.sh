#!/usr/bin/bash

println() {
    echo "================================================================================"
}

if [ ! -d tmp ]; then
    mkdir tmp || exit 1
fi

cd tmp || exit 1

testcase() {
    file=$1

    name=$(basename $file)
    name="${name%.*}"

    echo "Running test [$name]..."

    gcc -Wno-implicit-function-declaration $file || exit 1
    ../c $file ../test/$name.c > $name.actual
    ../c ../c.c $file ../test/$name.c > $name.actual
    ./a ../test/$name.c > $name.expect

    diff $name.expect $name.actual
    if [ $? != 0 ]
    then
        echo -e "\e[31mTest [$name] failed.\e[0m"
    else
        echo -e "\e[32mTest [$name] passed.\e[0m"
    fi
}

for file in ../test/*.c;
do
    testcase $file
done

exit 0
