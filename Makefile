FairRank: FairRank.cpp
	clang++ FairRank.cpp -o FairRank
run: FairRank
	@./FairRank
clean:
	@rm -f FairRank
.PHONY: clean run
