FairRank: FairRank.cpp
	clang++ FairRank.cpp -o FairRank
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
