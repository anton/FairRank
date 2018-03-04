#ifndef FAIRRANK_H
#define FAIRRANK_H

#ifdef __cplusplus
extern "C" {
#endif

struct fairrank_input
{
	char home[256];
	char away[256];
	int home_score;
	int away_score;
};

struct fairrank_output
{
	char name[256];
	double points;
	double certainty;
};

void fairrank_compute(struct fairrank_input *fr_input, size_t fr_input_len,
		      struct fairrank_output **fr_output, size_t *fr_output_len);

#ifdef __cplusplus
}
#endif

#endif /* FAIRRANK_H */
