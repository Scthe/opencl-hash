REM g++ -std=c++11

clang++ -c ^
  -stdlib=libstdc++ ^
  -std=c++1y ^
  -I "C:\programs\install\MinGW\include" ^
  -I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++" ^
  -I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++\mingw32" ^
  atom_test.cpp ^
  -o atom_test.exe ^
