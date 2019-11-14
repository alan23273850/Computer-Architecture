tc=(../testcases/*)
make clean
make

for folder in "${tc[@]}"; do
    echo $folder
    cp "$folder/dimage.bin" .
    cp "$folder/iimage.bin" .
    if [ "$folder" == "../testcases/array_3" ] || [ "$folder" == "../testcases/easy_swap" ] || [ "$folder" == "../testcases/example1" ] || [ "$folder" == "../testcases/example2" ] || [ "$folder" == "../testcases/example3" ] || [ "$folder" == "../testcases/fib_db" ]; then
        ./CMP
        diff snapshot.rpt "$folder/snapshot.rpt"
        diff report.rpt "$folder/report.rpt"
        rm *.rpt
    else
        ./CMP
        diff snapshot.rpt "$folder/snapshot.rpt"
        diff report.rpt "$folder/report1.rpt"
        rm *.rpt
        ./CMP 256 256 32 32 16 4 4 16 4 4
        diff snapshot.rpt "$folder/snapshot.rpt"
        diff report.rpt "$folder/report2.rpt"
        rm *.rpt
        ./CMP 512 1024 128 64 64 4 8 32 4 4
        diff snapshot.rpt "$folder/snapshot.rpt"
        diff report.rpt "$folder/report3.rpt"
        rm *.rpt
    fi
    rm *.bin
done

make clean
