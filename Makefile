__build_name = main
__test_name = test
__bench_name = bench
__graph_name = graph_bench

default:
	@echo "This is Make for Iridium."
	@echo "Build profiles supported are:"
	@echo "make build - compile the main game."
	@echo "make test - compile and run tests on all components."
	@echo "make bench - compile and run a benchmark with text readout."
	@echo "make graph - compile and run a benchmark and generate a callgraph."

build:
	g++ -std=c++2a -Ofast -Wall -Wextra -Werror -Wpedantic src/main.cpp -o target/$(__build_name)

# test:
# 	g++ -std=c++2a -Ofast -Wall -Wextra -Werror -Wpedantic test.cpp -o $(__test_name)
# 	./$(__test_name)

# bench:
# 	@echo "Running benchmark..."
# 	g++ -std=c++2a -Ofast -Wall -Wextra -Werror -Wpedantic bench.cpp -o $(__bench_name)
# 	./$(__bench_name)

graph_bench:
	@echo "Running callgraph benchmark..."
	g++ -std=c++2a -pg -Wall -Wextra -Werror -Wpedantic src/bench.cpp -o target/$(__graph_name)
	./target/$(__graph_name) 100 5000
	gprof ./target/$(__graph_name) | gprof2dot -s | dot -Tpng -o graph_bench.png

clean:
	rm -f target/$(__build_name)
	rm -f target/$(__test_name)
	rm -f target/$(__bench_name)
	rm -f target/$(__graph_name)
	rm -f gmon.out
	rm -f graph_bench.png
