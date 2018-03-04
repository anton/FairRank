#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fairrank.h"

int main()
{
	struct fairrank_output *fr_output = NULL;
	size_t fr_output_len = 0;

	size_t fr_input_len = 4;
	struct fairrank_input *fr_input = (struct fairrank_input *)malloc(fr_input_len * sizeof(struct fairrank_input));

	strncpy(&fr_input[0].home[0], "Federer", 255);
	strncpy(&fr_input[0].away[0], "Edberg", 255);
	fr_input[0].home_score = 2;
	fr_input[0].away_score = 0;

	strncpy(&fr_input[1].home[0], "Nickname1", 255);
	strncpy(&fr_input[1].away[0], "Nickname2", 255);
	fr_input[1].home_score = 3;
	fr_input[1].away_score = 3;

	strncpy(&fr_input[2].home[0], "Federer", 255);
	strncpy(&fr_input[2].away[0], "Anton", 255);
	fr_input[2].home_score = 0;
	fr_input[2].away_score = 5;

	strncpy(&fr_input[3].home[0], "Edberg", 255);
	strncpy(&fr_input[3].away[0], "Nickname2", 255);
	fr_input[3].home_score = 1;
	fr_input[3].away_score = 0;

	fairrank_compute(fr_input, fr_input_len, &fr_output, &fr_output_len);

	for (int i=0; i<fr_output_len; ++i)
	{
		printf("%16s %16.2lf %16.2lf\n", fr_output[i].name, fr_output[i].points, fr_output[i].certainty);
	}

	free(fr_input);
	free(fr_output);
}
