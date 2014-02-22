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

using namespace std;

typedef double real;

const real DIVG_DEFAULT = 5;
const real DIVG_MIN     = 1;
const real PC_DEFAULT   = 0.5; // We are quite uncertain of the players scores to begin with (any number between 0 and 1 should do here, just affects convergence)

inline real sqr(real x) { return x*x; }

// The names for the players and the id:s
map<string, int> g_nameToID;
map<int, string> g_IDToName;
int g_maxNameLength = 3;
int g_nplayers = 0;

int GetNameID(string name)
{
    if (g_nameToID.count(name) == 0)
    {
        g_IDToName[g_nplayers] = name;
        g_nameToID[name] = g_nplayers;
        g_nplayers++;
        g_maxNameLength = std::max(g_maxNameLength, (int)name.size());
    }
    return g_nameToID[name];
}

class Mat
{
public:
    Mat() : w(0), h(0) {}
    Mat(int w_, int h_) : w(w_), h(h_), vals(w*h, 0.0) {}

    real& operator()(int x, int y)       { return vals[x+y*w]; };
    real  operator()(int x, int y) const { return vals[x+y*w]; };

    void Print(ostream& os = cout)
    {
         int ew = std::max(7, g_maxNameLength+1);
         int p = 3;

         os << string(g_maxNameLength+2, ' ');
         for (int x=0; x<w; ++x)
              os << setw(g_maxNameLength+1) << g_IDToName[x];
         os << endl;

         for (int y=0; y<h; ++y)
         {
             os << setw(g_maxNameLength+2) << g_IDToName[y];
             for (int x=0; x<w; ++x)
             {
                 if ((*this)(x,y) == 0)
                    os << string(ew, ' ');
                 else
                     os << setw(ew) << setprecision(p) << (*this)(x,y);
             }
             os << endl;
         }
    }

private:
    int w, h;
    vector<real> vals;
};

typedef vector<real> Vec;

void Print(const Vec& v)
{
     int ew = 7;
     int p = 3;
     for (int x=0; x<v.size(); ++x)
         cout << setw(ew) << setprecision(p) << v[x];
}

// More Globals - yay!

Mat g_mp; // Player skill difference based on average match performance mano-e-mano
Mat g_mc; // The certainty of the above

const real CONVERGE_FACTOR = 0.45; // Should be in (0, 0.5]. 0.5 is the fastest, but may be a bit over-eager. A lower value relaxes the convergence

Vec UpdatePlayerScores(const Vec& ps, const Vec& pc)
{
    Vec ret(ps.size(), 0.0);
    for (int i=0; i<ps.size(); ++i)
    {
        real certSum = 0;
        for (int j=0; j<ps.size(); ++j)
        {
            real c = pc[j] * g_mc(i,j);
            certSum += c;
            ret[i] += c * (ps[j] + g_mp(i,j));
        }
        ret[i] /= certSum;
        ret[i] = ps[i] + CONVERGE_FACTOR * (ret[i] - ps[i]);
    }
    return ret;
}

void NormalizePlayerCertainty(Vec& pc)
{
    real sum = 0;
    for (int i=0; i<pc.size(); ++i)
        sum += pc[i];
    for (int i=0; i<pc.size(); ++i)
        pc[i] /= sum;
}

Vec UpdatePlayerCertainty(const Vec& ps, const Vec& pc)
{
    Vec ret(ps.size(), 0.0);
    for (int i=0; i<ps.size(); ++i)
    {
        //real certSum = 0;
        for (int j=0; j<ps.size(); ++j)
        {
            //certSum += mc(i,j);
            ret[i] += pc[j] * g_mc(i,j);
        }
        //ret[i] /= certSum;
        ret[i] = pc[i] + CONVERGE_FACTOR * (ret[i] - pc[i]);
    }
    NormalizePlayerCertainty(ret);
    return ret;
}

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

    //*
    fstream input("data/results");
    if (!input)
    {
       cerr << "input data missing!" << endl;
       exit(1);
    }
    /*/
    istream& input = cin;
    /**/

    map<int, map<int, vector<int> > > allScores;

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

           int id1 = GetNameID(n1);
           int id2 = GetNameID(n2);
           int scoreDiff = s[0] - s[1];

           allScores[id1][id2].push_back(+scoreDiff);
           allScores[id2][id1].push_back(-scoreDiff);
    }

    //cout << "Data read" << endl;
    //for (map<int, string>::const_iterator it = g_IDToName.begin(); it!=g_IDToName.end(); ++it)
    //    cout << setw(3) << it->first << " - " << it->second << endl;
    //cout << endl;

    Mat nmatches(g_nplayers, g_nplayers);
    Mat divgs(g_nplayers, g_nplayers);
    g_mp = Mat(g_nplayers, g_nplayers);
    g_mc = Mat(g_nplayers, g_nplayers);

    for (int i=0; i<g_nplayers; ++i)
    {
        for (int j=0; j<g_nplayers; ++j)
        {
            vector<int> scores = allScores[i][j];
            nmatches(i,j) = scores.size();

            if (scores.empty())
            {
                g_mp(i,j) = 0;
                g_mc(i,j) = 0;
                divgs(i,j) = 0;
            }
            else
            {
                real mean = 0;
                for (int k=0; k<scores.size(); ++k)
                    mean += (real)scores[k] / (real)scores.size();

                real divg;
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
                g_mp(i,j) = mean;
                g_mc(i,j) = log(scores.size()+1) / divg; // +1 to avoid log(1)==0
            }
        }
    }

    // Start calculating player scores
    Vec pp(g_nplayers, 0.0); // Player points
    Vec pc(g_nplayers, 1.0); // Player certainty
    NormalizePlayerCertainty(pc);

    const int ITERS = 30;

    for (int i=1; i<=ITERS; ++i)
    {
        Vec pp2 = UpdatePlayerScores(pp, pc);
        Vec pc2 = UpdatePlayerCertainty(pp, pc);

        pp = pp2;
        pc = pc2;

        if (i%5==0)
        {
            cout << "Scores after " << i << " iterations: "; Print(pp); cout << endl;
        }
    }

    cout << "(Make sure the above numbers converge)" << endl;

    Vec lowerBound(g_nplayers);
    for (int i=0; i<g_nplayers; ++i)
        lowerBound[i] = pp[i] - 1/(pc[i]*g_nplayers); // Very arbitarily choosen at the moment

    vector<int> ranking(g_nplayers);
    for (int i=0; i<g_nplayers; ++i)
        ranking[i] = i;
    std::sort(ranking.begin(), ranking.end(), RankComp(pp));

    cout << endl;
    cout << string(g_maxNameLength+2-4, ' ') << "Name    Rank   Certainty" << endl;
    for (int i=0; i<g_nplayers; ++i)
    {
        int n = ranking[i];
        cout << setw(g_maxNameLength+2) << g_IDToName[n] << setw(8) << pp[n] << setw(9) << pc[n] << endl;
    }

    std::ofstream ofs;
    ofs.open("data/stats");
    ofs << string(g_maxNameLength+2-4, ' ') << "Name    Rank   Certainty" << endl;
    for (int i=0; i<g_nplayers; ++i)
    {
        int n = ranking[i];
	ofs.width(g_maxNameLength+2);
	ofs << g_IDToName[n] ;
	ofs.width(8);
	ofs << pp[n];
	ofs.width(9);
	ofs << pc[n] << endl;
        //ofs << ofs.setw(g_maxNameLength+2) << g_IDToName[n] << ofs.setw(8) << pp[n] << ofs.setw(9) << pc[n] << endl;
    }
    ofs.close();

    cout << endl;

    cout << "Mean match points: (column beats row with an average of how many points)" << endl;
    g_mp.Print();
    cout << endl;

    cout << "Number of matches:" << endl;
    nmatches.Print();
    cout << endl;

    cout << "Divergence in match results:" << endl;
    divgs.Print();
    cout << endl;

    cout << "Certainty: (how much can we trust the mean match points)" << endl;
    g_mc.Print();
    cout << endl;
}
