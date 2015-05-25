REM add '.' to autocomplete !

clang ^
 -cc1 ^
 -x "c++" ^
 -fsyntax-only ^
 -I "C:\programs\install\MinGW\include" ^
 -I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++" ^
 -I "C:\programs\install\MinGW\lib\gcc\mingw32\4.7.2\include\c++\mingw32" ^
 "-code-completion-at=-:10:5" < "atom_test.cpp"
