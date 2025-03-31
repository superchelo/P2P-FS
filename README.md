# P2P-FS

simple peer 2 peer windows app that lets you transfer files between two windows computers on the same network

Server and client PCs must have windows network discoverability & file and printer sharing turned on for program to work.

computer that is sending the file(s) is the client\
computer that is receiving the file(s) is the server

Must link lws2_32 and use c++17 or above when compiling\
g++ -std=c++17 -o main.exe main.cpp server.cpp client.cpp -lws2_32

or use cmake

cmake -B build\
cd build\
cmake --build .
