g++ --version
g++ eqmap.cpp -Id:\code\boost\include -Ld:\code\boost\lib -lfreeglut -lglu32 -lopengl32 -Wl,--enable-auto-import -Wl,-subsystem,windows -Wall -O0 -s -o release/eqmap.exe
pause
