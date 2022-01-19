gcc -S ../cpu.c -o output1
gcc -S ../test.c -o output2
diff -a output1 output2 > ./test.asm
rm output1 && rm output2
cat test.asm
echo --------------------------------------
gcc -o test.out ../test.c
./test.out
echo Assembly and binary file are created.
