#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <cassert>
#include <algorithm>

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

inline double sqr(double x) { return x*x; }

class Players
{
      public:
	Players() : maxNameLength(3), nplayers(0) {}

	int getNameID(string name)
	{
		if (nameToID.count(name) == 0)
		{
			IDToName[nplayers] = name;
			nameToID[name] = nplayers;
			nplayers++;
			maxNameLength = std::max(maxNameLength, (int)name.size());
		}
		return nameToID[name];
	}

	string getName(int id)
	{
		return IDToName[id];
	}

	int getMaxNameLength() const { return maxNameLength; }
	int size() const { return nplayers; }

      private:
	int maxNameLength;
	int nplayers;
	// The names for the players and the id:s
	map<string, int> nameToID;
	map<int, string> IDToName;
};

class Mat
{
      public:
	Mat() : w(0), h(0) {}
	Mat(int w_, int h_) : w(w_), h(h_), vals(w * h, 0.0) {}
	double &operator()(int x, int y) { return vals[x + y * w]; };
	double operator()(int x, int y) const { return vals[x + y * w]; };
	int getW() const { return w; }
	int getH() const { return h; }
      private:
	int w, h;
	vector<double> vals;
};

typedef vector<double> Vec;

class Printer
{
	public:
	static void Print(Players &p, const Mat &m, ostream &os = cout)
	{
		int ew = std::max(7, p.getMaxNameLength() + 1);
		int pres = 3;
		os << string(p.getMaxNameLength() + 2, ' ');
		for (int x=0; x<m.getW(); ++x)
			os << setw(ew) << p.getName(x);
		os << endl;

		for (int y = 0; y < m.getH(); ++y)
		{
			os << setw(p.getMaxNameLength() + 2) << p.getName(y);
			for (int x = 0; x < m.getW(); ++x)
			{
				if (m(x, y) == 0)
					os << string(ew, ' ');
				else
					os << setw(ew) << setprecision(pres) << m(x, y);
			}
			os << endl;
		}
	}

	static void Print(const Vec& v)
	{
		int ew = 7;
		int p = 3;
		for (int x=0; x<v.size(); ++x)
			cout << setw(ew) << setprecision(p) << v[x];
	}
};

class Updater
{
      public:
        Updater(Mat &mp, Mat &mc) : mp(mp), mc(mc) {}

	Vec UpdatePlayerScores(const Vec &ps, const Vec &pc)
	{
		Vec ret(ps.size(), 0.0);
		for (int i = 0; i < ps.size(); ++i)
		{
			double certSum = 0;
			for (int j = 0; j < ps.size(); ++j)
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
		for (int i = 0; i < ps.size(); ++i)
		{
			for (int j = 0; j < ps.size(); ++j)
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
		for (int i = 0; i < pc.size(); ++i)
			sum += pc[i];
		for (int i = 0; i < pc.size(); ++i)
			pc[i] /= sum;
	}

      private:
	Mat &mp; // Player skill difference based on average match performance mano-e-mano
	Mat &mc; // The certainty of the above

	const double CONVERGE_FACTOR = 0.45; // Should be in (0, 0.5]. 0.5 is the fastest, but may be a bit over-eager. A lower value relaxes the convergence
};

class RankComp
{
	public:
		RankComp(const Vec& v) : m_scores(v) {}

		bool operator()(int p0, int p1) { return m_scores[p0] > m_scores[p1]; }
	private:
		Vec m_scores;
};

int main(int argc, char *argv[])
{
	// Read input
	fstream input("data/results");
	if (!input)
	{
		cerr << "input data missing!" << endl;
		exit(1);
	}

	map<int, map<int, vector<int> > > allScores;

	const double DIVG_DEFAULT = 5;
	const double DIVG_MIN     = 1;

	Players players;
	Mat mp; // Player skill difference based on average match performance mano-e-mano
	Mat mc; // The certainty of the above

	for(;;)
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

		int id1 = players.getNameID(n1);
		int id2 = players.getNameID(n2);
		int scoreDiff = s[0] - s[1];

		allScores[id1][id2].push_back(+scoreDiff);
		allScores[id2][id1].push_back(-scoreDiff);
	}

	Mat nmatches(players.size(), players.size());
	Mat divgs(players.size(), players.size());
	mp = Mat(players.size(), players.size());
	mc = Mat(players.size(), players.size());

	for (int i=0; i<players.size(); ++i)
	{
		for (int j=0; j<players.size(); ++j)
		{
			vector<int> scores = allScores[i][j];
			nmatches(i,j) = scores.size();

			if (scores.empty())
			{
				mp(i,j) = 0;
				mc(i,j) = 0;
				divgs(i,j) = 0;
			}
			else
			{
				double mean = 0;
				for (int k=0; k<scores.size(); ++k)
					mean += (double)scores[k] / (double)scores.size();

				double divg;
				if (scores.size() <= 1)
				{
					divg = DIVG_DEFAULT;
				}
				else
				{
					divg = 0;
					for (int k=0; k<scores.size(); ++k)
						divg += sqr(scores[k]-mean) / (scores.size() - 1);

					divg = sqrt(divg);

					divg = std::max(divg, DIVG_MIN);
				}

				divgs(i,j) = divg;
				mp(i,j) = mean;
				mc(i,j) = log(scores.size()+1) / divg; // +1 to avoid log(1)==0
			}
		}
	}

	// Start calculating player scores
	Vec pp(players.size(), 0.0); // Player points
	Vec pc(players.size(), 1.0); // Player certainty
	Updater::NormalizePlayerCertainty(pc);

	const int ITERS = 30;

	Updater u(mp, mc);
	for (int i=1; i<=ITERS; ++i)
	{
		Vec pp2 = u.UpdatePlayerScores(pp, pc);
		Vec pc2 = u.UpdatePlayerCertainty(pp, pc);

		pp = pp2;
		pc = pc2;

		if (i%5==0)
		{
			cout << "Scores after " << setw(2) << i << " iterations: "; Printer::Print(pp); cout << endl;
		}
	}

	cout << "(Make sure the above numbers converge)" << endl;

	Vec lowerBound(players.size());
	for (int i=0; i<players.size(); ++i)
		lowerBound[i] = pp[i] - 1/(pc[i]*players.size()); // Very arbitarily choosen at the moment

	vector<int> ranking(players.size());
	for (int i=0; i<players.size(); ++i)
		ranking[i] = i;
	std::sort(ranking.begin(), ranking.end(), RankComp(pp));

	std::ofstream ofs;
	ofs.open("data/stats");
	ofs << string(players.getMaxNameLength()+2-4, ' ') << "Name                Rank           Certainty" << endl;
	for (int i=0; i<players.size(); ++i)
	{
		int n = ranking[i];
		ofs.width(players.getMaxNameLength()+2);
		ofs << players.getName(n) ;
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
	cout << string(players.getMaxNameLength()+2-4, ' ') << "### Leaderboard ###" << endl;
	cout << endl;
	cout << "  #" << string(players.getMaxNameLength()+2-4, ' ') << "Name    Rank   Certainty" << endl;
	for (int i=0; i<players.size(); ++i)
	{
		int n = ranking[i];
		cout << setw(3) << i+1 << setw(players.getMaxNameLength()+2) << players.getName(n) << setw(8) << setprecision(2) << pp[n] << setw(12) << pc[n] << endl;
	}
}
