#**********************
#*
#* Progam Name: MP1. Membership Protocol.
#*
#* Current file: run.sh
#* About this file: Submission shell script.
#* 
#***********************
#!/bin/sh
sudo mkdir grade-dir
cd grade-dir
sudo wget https://spark-public.s3.amazonaws.com/cloudcomputing/assignments/mp1/mp1.zip
sudo unzip mp1.zip
sudo cp ../mp1_node.* .
make clean > /dev/null
make > /dev/null
case $1 in
	1) ./app testcases/singlefailure.conf > /dev/null;;
	2) ./app testcases/multifailure.conf > /dev/null;;
	3) ./app testcases/msgdropsinglefailure.conf > /dev/null;;
	*) echo "Please enter a valid option";;
esac
cp dbg.log ../
cd ..
sudo rm -rf grade-dir
