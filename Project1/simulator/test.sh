tc=(../testcases/*)
make clean
make

for folder in "${tc[@]}"; do
    cp "$folder/dimage.bin" .
    cp "$folder/iimage.bin" .
    ./single_cycle
    diff snapshot.rpt "$folder/snapshot.rpt"
    diff error_dump.rpt "$folder/error_dump.rpt"
    rm *.bin *.rpt
done

make clean
