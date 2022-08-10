all:
	g++ -std=c++20 src/*.cpp -o bbserv
clean:
	rm -rf *.log *.pid bbserv
