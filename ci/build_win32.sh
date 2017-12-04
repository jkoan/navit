apt-get update && apt-get install -y mingw-w64 binutils-mingw-w64 default-jdk nsis

mkdir win32
pushd win32
cmake -Dbinding/python:BOOL=FALSE -DSAMPLE_MAP=n -DCMAKE_TOOLCHAIN_FILE=../Toolchain/mingw32.cmake ../ && make -j $(nproc --all)  && make -j $(nproc --all) package
popd

cp win32/*.exe $CIRCLE_ARTIFACTS/
