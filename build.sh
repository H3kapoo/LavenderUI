set -xe

CXX=$(which g++-14) cmake -B bbuild -GNinja .
cmake --build bbuild

cd lib
cp lavenderui ../lavenderui
cd ..
./lavenderui