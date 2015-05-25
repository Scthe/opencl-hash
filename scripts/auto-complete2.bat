REM add '.' to autocomplete !

clang ^
 -cc1 ^
 -x "c++" ^
 -std=c++1y ^
 -fsyntax-only ^
 -I "C:\programs\install\MinGW\include" ^
 -I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++" ^
 -I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++\mingw32" ^
 "-code-completion-at=-:16:7" < "atom_test.cpp"
