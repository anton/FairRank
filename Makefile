CC=clang
CXX=clang++

CXXFLAGS=-O3 -pedantic-errors -Werror -Weverything -std=c++11 -Wno-c++98-compat

FairRank: FairRank.cpp
	$(CXX) $(CXXFLAGS) -DTEST $< -o $@

FairRank.o: FairRank.cpp
	$(CXX) $(CXXFLAGS) -c -fpic $< -o $@

libfairrank.so: FairRank.o fairrank.h
	$(CXX) $(CXXFLAGS) -shared -lc -o $@ FairRank.o

testrun: test
	LD_LIBRARY_PATH=./ ./test

test: test.c libfairrank.so
	$(CC) $(CFLAGS) test.c -o $@ -L. -lfairrank

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
	@rm -f FairRank test *.o *.so

.PHONY: clean run testrun
