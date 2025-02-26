HelloWorld.exe: HelloWorld.cpp
	g++ HelloWorld.cpp -o HelloWorld.exe

clean:
	rm HelloWorld.exe
