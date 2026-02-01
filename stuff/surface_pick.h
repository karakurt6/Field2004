struct Sample
{
  float sparam;
  float xcoord;
  float ycoord;
};

typedef std::vector<Sample> Sample_list;

struct Well
{
	char name[32];
	long bore;
	Sample org;
	Sample_list traj;
};

typedef std::list<Well> Well_list;

bool surface_pick(const char* prefix, const char* surf, bool roof, Well_list& acc);

bool profile_pick(const char* prefix, const char* surf, bool roof, \
  int n, const long* bore, const double* xx, const double* yy, Well_list& acc);
