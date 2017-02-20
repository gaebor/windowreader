for file in test/test*.txt; do ./wr < $file | diff test/`basename $file .txt`.result - ; done
