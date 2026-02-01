class WellMatch
{
  typedef std::valarray < CFieldDoc::InfoItem* > infoarray;
  typedef std::valarray < CFieldDoc::PerfItem* > perfarray;
  typedef std::valarray < CFieldDoc::ProdItem* > prodarray;
  typedef std::valarray < CFieldDoc::PumpItem* > pumparray;

  struct Well
  {
    char name[16];
    long bore;
    vertex_type ver;
    edge_type t_edge;
    edge_type p_opened;
		edge_type p_closed;
    infoarray info;
    perfarray perf;
    prodarray prod;
    pumparray pump;

		bool has_logtrack;
		double x0, y0;
		double dx, dy;
		double x1, x2;
		double y1, y2;
		float sp_range[2];
		float il_range[2];
		float pz_range[2];
		edge_type sp_edge;
		edge_type il_edge;
		edge_type pz_edge;
  };

  typedef std::list < Well > well_list;

  struct Page
  {
    well_list w_list;
  };

  typedef std::list < Page > page_list;

  page_list p_list;
  page_list::iterator p_iter;

public:
  WellMatch();
  void new_page(double x_range[2], double y_range[2], int first, int last, const float *dist, \
    const std::map<int, long>& bore, CActField* pDoc, CCrossSection* pSec);
  void first_page();
  void next_page();
  void plot_page(psstream& out);
};
