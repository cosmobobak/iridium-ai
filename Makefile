
build:
	g++ -std=c++2a -Ofast -Wall -Wextra -Werror -Wpedantic src/main.cpp -o target/main

test:
	g++ -std=c++2a -Ofast -Wall -Wextra -Werror -Wpedantic src/main.cpp -o target/test
	./target/test

clean:
	rm -f target/test
	rm -f target/main
