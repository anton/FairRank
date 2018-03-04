FairRank: FairRank.cpp
	clang++ -O3 -pedantic-errors -Werror -Weverything -std=c++11 -Wno-c++98-compat FairRank.cpp -DTEST -o FairRank
FairRank.o: FairRank.cpp
	clang++ -c -O0 -pedantic-errors -Werror -Weverything -std=c++11 -Wno-c++98-compat FairRank.cpp -o FairRank.o
test: test.c FairRank.o
	clang++ -c test.c -o test.o
	clang++ FairRank.o test.o -o test
run: FairRank data/results
	@./FairRank
data/results:
	@mkdir -p data
	@echo foo bar 3-2 >  data/results
	@echo foo bar 3-1 >> data/results
	@echo foo baz 0-1 >> data/results
	@echo bar baz 0-3 >> data/results
	@echo Sample results generated to data/results
clean:
	@rm -f FairRank
.PHONY: clean run
