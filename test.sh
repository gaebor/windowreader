for file in test/test*.txt
do
    ./wr < $file | diff $file.result -
    if [[ $? -ne 0 ]]
    then
        echo error in $file
        exit 1
    fi
done

echo OK
