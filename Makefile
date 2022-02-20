
all: vkb

vkb: main.cpp vkeyboard.cpp
	g++ -o vkb main.cpp vkeyboard.cpp

clean: 
	rm vkb