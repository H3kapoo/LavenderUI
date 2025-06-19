set -xe

CXX=$(which g++-14) cmake -B artifacts -GNinja .
cmake --build artifacts

cd debug
mv toying ../toying
cd ..
rm -rf debug/
./toying