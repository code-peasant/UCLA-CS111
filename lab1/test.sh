#!/usr/local/cs/bin/bash

tmp=temp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

#test case 1
#This test case checks if --rdonly, --wronly, and --command works
cat > input1.txt <<'EOF'
e
e
d
d
d
c
c
c
c
EOF

cat > result1.txt <<'EOF'
c
d
e
EOF

touch output1.txt

cat > test1.sh <<'EOF'
#!/usr/local/cs/bin/bash
../simpsh \
--rdonly input1.txt \
--wronly output1.txt \
--command 0 1 2 sort -u \
--wait \
> /dev/null
EOF

chmod u+x test1.sh

./test1.sh

diff -u result1.txt output1.txt || exit

# test case 2
# This test case checks if --verbose works
cat > input2.txt << 'EOF'
this is a simple test
EOF

touch output2.txt

touch std_out.txt

cat > result2.txt << 'EOF'
THIS IS A SIMPLE TEST
EOF

cat > real_stdout.txt <<EOF
--rdonly input2.txt
--wronly output2.txt
--command 0 1 2 tr a-z A-Z
--wait
0 tr a-z A-Z
EOF

cat > test2.sh <<EOF
#!/usr/local/cs/bin/bash
../simpsh \
--verbose \
--rdonly input2.txt \
--wronly output2.txt \
--command 0 1 2 tr a-z A-Z \
--wait > ./std_out.txt 
EOF

chmod u+x ./test2.sh

./test2.sh

diff -u result2.txt output2.txt || exit

diff -u std_out.txt real_stdout.txt || exit

#test case 3
#This test case checks if multiple --command works

echo > output2.txt

cat > result2.txt <<EOF
THIS IS A SIMPLE TEST
EOF

cat > real_stdout.txt <<EOF
--rdonly input2.txt
--wronly output2.txt
--command 0 1 2 tr a-z A-Z
--wait
0 tr a-z A-Z
EOF

cat > test3.sh <<EOF
#!/usr/local/cs/bin/bash
../simpsh \
--verbose \
--rdonly input2.txt \
--wronly output2.txt \
--command 0 1 2 tr a-z A-Z \
--wait\
> stdout.txt
EOF

chmod u+x ./test3.sh

./test3.sh

diff -u result2.txt output2.txt || exit
diff -u stdout.txt real_stdout.txt || exit
) || exit

rm -fr "$tmp"
