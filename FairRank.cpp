#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using std::endl;
using std::cout;
using std::setw;
using std::string;
using std::setprecision;
using std::vector;
using std::istringstream;
using std::stringstream;
using std::map;
using std::cerr;
using std::fstream;
using std::ostream;

inline double sqr(double x) { return x * x; }

class Players
{
      public:
	Players() : maxNameLength(3), nplayers(0) {}

	size_t getNameID(string name)
	{
		if (nameToID.count(name) == 0)
		{
			IDToName[nplayers] = name;
			nameToID[name] = nplayers;
			nplayers++;
			maxNameLength = std::max(maxNameLength, name.size());
		}
		return nameToID[name];
	}

	string getName(size_t id) { return IDToName[id]; }

	size_t getMaxNameLength() const { return maxNameLength; }
	size_t size() const { return nplayers; }

      private:
	size_t maxNameLength;
	size_t nplayers;
	// The names for the players and the id:s
	map<string, size_t> nameToID;
	map<size_t, string> IDToName;
};

class Mat
{
      public:
	Mat() : w(0), h(0) {}
	Mat(size_t w_, size_t h_) : w(w_), h(h_), vals(w * h, 0.0) {}
	double &operator()(size_t x, size_t y) { return vals[x + y * w]; }
	double operator()(size_t x, size_t y) const { return vals[x + y * w]; }
	size_t getW() const { return w; }
	size_t getH() const { return h; }
      private:
	size_t w, h;
	vector<double> vals;
};

typedef vector<double> Vec;

class Printer
{
      public:
	static void Print(Players &p, const Mat &m, ostream &os = cout)
	{
		const size_t ew = std::max(static_cast<size_t>(7), p.getMaxNameLength() + 1);
		const int ewi = static_cast<int>(ew);
		int pres = 3;
		os << string(p.getMaxNameLength() + 2, ' ');
		for (size_t x = 0; x < m.getW(); ++x)
			os << setw(ewi) << p.getName(x);
		os << endl;

		for (size_t y = 0; y < m.getH(); ++y)
		{
			os << setw(static_cast<int>(p.getMaxNameLength() + 2)) << p.getName(y);
			for (size_t x = 0; x < m.getW(); ++x)
			{
				if (m(x, y) < 0.00001)
					os << string(ew, ' ');
				else
					os << setw(ewi) << setprecision(pres) << m(x, y);
			}
			os << endl;
		}
	}

	static void Print(const Vec &v)
	{
		int ew = 7;
		int p = 3;
		for (size_t x = 0; x < v.size(); ++x)
			cout << setw(ew) << setprecision(p) << v[x];
	}
};

class Updater
{
      public:
	Updater(Mat &mpm, Mat &mcm) : mp(mpm), mc(mcm) {}

	Vec UpdatePlayerScores(const Vec &ps, const Vec &pc)
	{
		Vec ret(ps.size(), 0.0);
		for (size_t i = 0; i < ps.size(); ++i)
		{
			double certSum = 0;
			for (size_t j = 0; j < ps.size(); ++j)
			{
				double c = pc[j] * mc(i, j);
				certSum += c;
				ret[i] += c * (ps[j] + mp(i, j));
			}
			ret[i] /= certSum;
			ret[i] = ps[i] + CONVERGE_FACTOR * (ret[i] - ps[i]);
		}
		return ret;
	}

	Vec UpdatePlayerCertainty(const Vec &ps, const Vec &pc)
	{
		Vec ret(ps.size(), 0.0);
		for (size_t i = 0; i < ps.size(); ++i)
		{
			for (size_t j = 0; j < ps.size(); ++j)
			{
				ret[i] += pc[j] * mc(i, j);
			}
			ret[i] = pc[i] + CONVERGE_FACTOR * (ret[i] - pc[i]);
		}
		NormalizePlayerCertainty(ret);
		return ret;
	}

	static void NormalizePlayerCertainty(Vec &pc)
	{
		double sum = 0;
		for (size_t i = 0; i < pc.size(); ++i)
			sum += pc[i];
		for (size_t i = 0; i < pc.size(); ++i)
			pc[i] /= sum;
	}

      private:
	Mat &mp; // Player skill difference based on average match performance mano-e-mano
	Mat &mc; // The certainty of the above

	const double CONVERGE_FACTOR = 0.45; // Should be in (0, 0.5]. 0.5 is the fastest, but may be a bit over-eager.
	                                     // A lower value relaxes the convergence
};

class RankComp
{
      public:
	RankComp(const Vec &v) : m_scores(v) {}

	bool operator()(size_t p0, size_t p1) { return m_scores[p0] > m_scores[p1]; }
      private:
	Vec m_scores;
};

int main()
{
	// Read input
	fstream input("data/results");
	if (!input)
	{
		cerr << "input data missing!" << endl;
		exit(1);
	}

	map<size_t, map<size_t, vector<int>>> allScores;

	const double DIVG_DEFAULT = 5;
	const double DIVG_MIN = 1;

	Players players;
	Mat mp; // Player skill difference based on average match performance mano-e-mano
	Mat mc; // The certainty of the above

	for (;;)
	{
		string str;
		if (!getline(input, str))
			break;
		if (str.size() == 0 || str[0] == '#')
			continue;

		stringstream ss(str);
		string n1, n2, score;
		int s[2];
		ss >> n1 >> n2 >> score;

		size_t pos = score.find("-");
		string token = score.substr(0, pos);
		istringstream(token) >> s[0];
		score.erase(0, pos + 1);
		istringstream(score) >> s[1];

		size_t id1 = players.getNameID(n1);
		size_t id2 = players.getNameID(n2);
		int scoreDiff = s[0] - s[1];

		allScores[id1][id2].push_back(+scoreDiff);
		allScores[id2][id1].push_back(-scoreDiff);
	}

	Mat nmatches(players.size(), players.size());
	Mat divgs(players.size(), players.size());
	mp = Mat(players.size(), players.size());
	mc = Mat(players.size(), players.size());

	for (size_t i = 0; i < players.size(); ++i)
	{
		for (size_t j = 0; j < players.size(); ++j)
		{
			vector<int> scores = allScores[i][j];
			nmatches(i, j) = scores.size();

			if (scores.empty())
			{
				mp(i, j) = 0;
				mc(i, j) = 0;
				divgs(i, j) = 0;
			}
			else
			{
				double mean = 0;
				for (size_t k = 0; k < scores.size(); ++k)
					mean += static_cast<double>(scores[k]) / static_cast<double>(scores.size());

				double divg;
				if (scores.size() <= 1)
				{
					divg = DIVG_DEFAULT;
				}
				else
				{
					divg = 0;
					for (size_t k = 0; k < scores.size(); ++k)
						divg += sqr(scores[k] - mean) / (scores.size() - 1);

					divg = sqrt(divg);

					divg = std::max(divg, DIVG_MIN);
				}

				divgs(i, j) = divg;
				mp(i, j) = mean;
				mc(i, j) = log(scores.size() + 1) / divg; // +1 to avoid log(1)==0
			}
		}
	}

	// Start calculating player scores
	Vec pp(players.size(), 0.0); // Player points
	Vec pc(players.size(), 1.0); // Player certainty
	Updater::NormalizePlayerCertainty(pc);

	const int ITERS = 30;

	Updater u(mp, mc);
	for (int i = 1; i <= ITERS; ++i)
	{
		Vec pp2 = u.UpdatePlayerScores(pp, pc);
		Vec pc2 = u.UpdatePlayerCertainty(pp, pc);

		pp = pp2;
		pc = pc2;

		if (i % 5 == 0)
		{
			cout << "Scores after " << setw(2) << i << " iterations: ";
			Printer::Print(pp);
			cout << endl;
		}
	}

	cout << "(Make sure the above numbers converge)" << endl;

	Vec lowerBound(players.size());
	for (size_t i = 0; i < players.size(); ++i)
		lowerBound[i] = pp[i] - 1 / (pc[i] * players.size()); // Very arbitarily choosen at the moment

	vector<size_t> ranking(players.size());
	for (size_t i = 0; i < players.size(); ++i)
		ranking[i] = i;
	std::sort(ranking.begin(), ranking.end(), RankComp(pp));

	std::ofstream ofs;
	ofs.open("data/stats");
	ofs << string(players.getMaxNameLength() + 2 - 4, ' ') << "Name                Rank           Certainty"
	    << endl;
	for (size_t i = 0; i < players.size(); ++i)
	{
		size_t n = ranking[i];
		ofs.width(static_cast<int>(players.getMaxNameLength() + 2));
		ofs << players.getName(n);
		ofs.width(20);
		ofs << pp[n];
		ofs.width(20);
		ofs << pc[n] << endl;
	}
	ofs.close();

	cout << endl;

	cout << "Mean match points: (column beats row with an average of how many points)" << endl;
	Printer::Print(players, mp);
	cout << endl;

	cout << "Number of matches:" << endl;
	Printer::Print(players, nmatches);
	cout << endl;

	cout << "Divergence in match results:" << endl;
	Printer::Print(players, divgs);
	cout << endl;

	cout << "Certainty: (how much can we trust the mean match points)" << endl;
	Printer::Print(players, mc);
	cout << endl;

	cout << endl;
	cout << string(players.getMaxNameLength() + 2 - 4, ' ') << "### Leaderboard ###" << endl;
	cout << endl;
	cout << "  #" << string(players.getMaxNameLength() + 2 - 4, ' ') << "Name    Rank   Certainty" << endl;
	for (size_t i = 0; i < players.size(); ++i)
	{
		size_t n = ranking[i];
		cout << setw(3) << i + 1 << setw(static_cast<int>(players.getMaxNameLength() + 2)) << players.getName(n)
		     << setw(8) << setprecision(2) << pp[n] << setw(12) << pc[n] << endl;
	}
}
