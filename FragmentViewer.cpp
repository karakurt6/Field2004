// FragmentViewer.cpp : implementation file
//

#include "stdafx.h"
#include "Field.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "FragmentViewer.h"

#include <algorithm>
#include <valarray>
#include <limits>

#include <Inventor\Win\Viewers\SoWinExaminerViewer.h>
#include <Inventor\Nodes\SoSeparator.h>
#include <Inventor\Nodes\SoCone.h>
#include <Inventor\Nodes\SoTransform.h>
#include <Inventor\Nodes\SoText2.h>
#include <Inventor\Nodes\SoCylinder.h>
#include <Inventor\Nodes\SoMaterial.h>
#include <Inventor\Nodes\SoEnvironment.h>
#include <Inventor\SoOffscreenRenderer.h>
#include <Inventor\SbViewportRegion.h>
#include <DataViz\Graph\PoAutoCubeAxis.h>
#include <DataViz\Nodes\PoDomain.h>

SoSeparator* BuildScene(CActField* pDoc, long bore, double aperture, double z_factor)
{
	double p[3];
	int n = pDoc->Traj(bore, 1, p);
	double x1 = p[0] - aperture;
	double x2 = p[0] + aperture;
	double y1 = p[1] - aperture;
	double y2 = p[1] + aperture;
	if (x1 < pDoc->x1)
	{
		double dx = pDoc->x1 - x1;
		x1 = pDoc->x1;
		if (x2 + dx < pDoc->x2)
		{
			x2 = x2 + dx;
		}
		else
		{
			x2 = pDoc->x2;
		}
	}
	if (y1 < pDoc->y1)
	{
		double dy = pDoc->y1 - y1;
		y1 = pDoc->y1;
		if (y2 + dy < pDoc->y2)
		{
			y2 = y2 + dy;
		}
		else
		{
			y2 = pDoc->y1;
		}
	}
	if (x2 > pDoc->x2)
	{
		double dx = x2 - pDoc->x2;
		x2 = pDoc->x2;
		if (x1 - dx > pDoc->x1)
		{
			x1 = x1 - dx;
		}
		else
		{
			x1 = pDoc->x1;
		}
	}
	if (y2 > pDoc->y2)
	{
		double dy = y2 - pDoc->y2;
		y2 = pDoc->y2;
		if (y2 - dy > pDoc->y1)
		{
			y1 = y1 - dy;
		}
		else
		{
			y1 = pDoc->y1;
		}
	}

	double u1 = pDoc->cx * (x1 - pDoc->x1) / (pDoc->x2 - pDoc->x1);
	double u2 = pDoc->cx * (x2 - pDoc->x1) / (pDoc->x2 - pDoc->x1);
	double v1 = pDoc->cy * (y1 - pDoc->y1) / (pDoc->y2 - pDoc->y1);
	double v2 = pDoc->cy * (y2 - pDoc->y1) / (pDoc->y2 - pDoc->y1);

	CActField::Point a = { u1, v1 }, b = { u2, v2 };
	long bb[128];
	int nb = std::min(pDoc->WindowQuery(a, b, 128, bb), 128);

	SoSeparator* root = new SoSeparator;

	SoSeparator* sep1 = new SoSeparator;
	root->addChild(sep1);

	SoTransform* xform1 = new SoTransform;
	sep1->addChild(xform1);
	xform1->rotation = SbRotation(SbVec3f(0, 1, 0), SbVec3f(0, -1, 0));

	PoDomain* dom = new PoDomain;
	sep1->addChild(dom);
	double z1 = std::numeric_limits<double>::max();
	double z2 = -z1;

	dom->min.setValue(-500, -500, -100);
	dom->max.setValue(500, 500, 0);

  SoEnvironment *env = new SoEnvironment;
	env->ambientIntensity.setValue(1.5);
	root->addChild(env);

	PoAutoCubeAxis *cube = new PoAutoCubeAxis(SbVec3f(-500, -500, -100), SbVec3f(500, 500, 0), \
		PoAutoCubeAxis::LINEAR, PoAutoCubeAxis::LINEAR, PoAutoCubeAxis::LINEAR, "X", "Y", "Z");
	cube->set("backgroundFaceApp.material", "diffuseColor 1 0 0");
	cube->isBackgroundFacesVisible = TRUE;
	sep1->addChild(cube);

	for (int i = 0; i < nb; ++i)
	{
		int np = pDoc->Traj(bb[i], 0, 0);
		if (np > 0)
		{
			std::valarray<double> md(np);
			std::valarray<double> pp(3*np);
			pDoc->Traj(bb[i], np, &pp[0]);
			SoSeparator *child = new SoSeparator;
			root->addChild(child);

			SoSeparator* shape = new SoSeparator;
			child->addChild(shape);
			SoTransform* xform = new SoTransform;
			shape->addChild(xform);
			xform->translation = SbVec3f((float) pp[0], (float) pp[1], (float) -pp[2]+25);
			xform->rotation = SbRotation(SbVec3f(0, 1, 0), SbVec3f(0, 0, 1));
			SoCone* cone = new SoCone;
			shape->addChild(cone);
			cone->bottomRadius = 20;
			cone->height = 50;
			if (z1 > -pp[2]) z1 = -pp[2];
			if (z2 < -pp[2]) z2 = -pp[2];

			char name[16];
			xform = new SoTransform;
			shape->addChild(xform);
			xform->translation = SbVec3f(25, 0, 0);
			LookupWellName(pDoc, bb[i], name);
			SoText2 *text = new SoText2;
			shape->addChild(text);
			text->string.set1Value(0, name);

			md[0] = 0.0;
			for (int j = 1; j < np; ++j)
			{
				double dx = pp[3*j] - pp[3*j-3];
				double dy = pp[3*j+1] - pp[3*j-2];
				double dz = pp[3*j+2] - pp[3*j-1];
				md[j] = md[j-1] + sqrt(dx*dx + dy*dy + dz* dz);
			}

			SbVec3f prev(0, 0, 0);
			for (int j = 1; j < np; ++j)
			{
				float dx = float(pp[3*j] - pp[0]);
				float dy = float(pp[3*j+1] - pp[1]);
				float dz = float((pp[3*j+2] - pp[2]) * z_factor);
				SbVec3f curr(dx, dy, -dz);
				SbVec3f diff = curr - prev;
				diff.normalize();

				/*
				do
				{
					if (j+1 == np)
						break;

					dx = float(pp[3*j+3] - pp[0]);
					dy = float(pp[3*j+4] - pp[1]);
					dz = float((pp[3*j+5] - pp[2]) * z_factor);

					SbVec3f next(dx, dy, -dz);
					SbVec3f diff2 = next - prev;
					diff2.normalize();

					if (diff.dot(diff2) < 0.999)
						break;

					curr = next;
					++j;
				}
				while (1);
				*/

				diff = curr - prev;
				float dd = diff.normalize();

				shape = new SoSeparator;
				child->addChild(shape);
				xform = new SoTransform;
				shape->addChild(xform);
				xform->translation = SbVec3f((float) pp[0], (float) pp[1], (float) -pp[2]) + 0.5 * (curr + prev);
				xform->rotation = SbRotation(SbVec3f(0, 1, 0), diff);
				SoCylinder *cylinder = new SoCylinder;
				shape->addChild(cylinder);
				cylinder->radius = 6;
				cylinder->height = dd;

				prev = curr;
			}

			// P E R F O R A T I O N
			int n_per = pDoc->Perf(bb[i], 0, 0);
			if (n_per > 0)
			{
				SoMaterial* mat = new SoMaterial;
				mat->diffuseColor.setValue(1.0, 0.0, 0.0);

				std::valarray<CActField::PerfItem*> per(n_per);
				pDoc->Perf(bb[i], n_per, &per[0]);
				for (int k = 0; k < n_per; ++k)
				{
					double *dist = &md[0];
					double d1 = per[k]->perf_upper;
					double d2 = per[k]->perf_lower;

					if (d1 < 0.0)
						d1 = 0.0;
					if (d1 > dist[np-1])
						d1 = dist[np-1];
					if (d2 < 0.0)
						d2 = 0.0;
					if (d2 > dist[np-1])
						d2 = dist[np-1];

					int n1 = std::lower_bound(dist, dist+np, d1) - dist;
					double x1 = pp[3*n1];
					double y1 = pp[3*n1+1];
					double z1 = pp[3*n1+2];
					if (d1 < dist[n1])
					{
						double t = (d1 - dist[n1-1]) / (dist[n1] - dist[n1-1]);
						ASSERT(0 <= t && t <= 1.0);
						x1 = (1 - t) * pp[3*(n1-1)] + t * x1;
						y1 = (1 - t) * pp[3*(n1-1)+1] + t * y1;
						z1 = (1 - t) * pp[3*(n1-1)+2] + t * z1;
					}

					SbVec3f prev = SbVec3f((float) (x1 - pp[0]), (float) (y1 - pp[1]), (float) -((z1 - pp[2]) * z_factor));

					int n2 = std::lower_bound(dist, dist+np, d2) - dist;
					double x2 = pp[3*n2];
					double y2 = pp[3*n2+1];
					double z2 = pp[3*n2+2];
					if (d2 < dist[n2])
					{
						double t = (d2 - dist[n2-1]) / (dist[n2] - dist[n2-1]);
						ASSERT(0 <= t && t <= 1.0);
						x2 = (1 - t) * pp[3*(n2-1)] + t * x2;
						y2 = (1 - t) * pp[3*(n2-1)+1] + t * y2;
						z2 = (1 - t) * pp[3*(n2-1)+2] + t * z2;
					}

					for (int n = n1; n < n2; ++n)
					{
						SbVec3f curr((float) (pp[3*n] - pp[0]), (float) (pp[3*n+1] - pp[1]), (float) (z_factor * -(pp[3*n+2]-pp[2])));
						SbVec3f diff = curr - prev;
						float norm = diff.normalize();

						shape = new SoSeparator;
						child->addChild(shape);
						shape->addChild(mat);
						xform = new SoTransform;
						shape->addChild(xform);
						xform->translation = SbVec3f((float) pp[0], (float) pp[1], (float) -pp[2]) + 0.5 * (curr + prev);
						xform->rotation = SbRotation(SbVec3f(0, 1, 0), diff);
						SoCylinder *cylinder = new SoCylinder;
						shape->addChild(cylinder);
						cylinder->radius = 10;
						cylinder->height = norm;

						prev = curr;
					}

					{
						SbVec3f curr = SbVec3f((float) (x2 - pp[0]), (float) (y2 - pp[1]), (float) (-z_factor * (z2 - pp[2])));
						SbVec3f diff = curr - prev;
						float norm = diff.normalize();

						shape = new SoSeparator;
						child->addChild(shape);
						shape->addChild(mat);
						xform = new SoTransform;
						shape->addChild(xform);
						xform->translation = SbVec3f((float) pp[0], (float) pp[1], (float) -pp[2]) + 0.5 * (curr + prev);
						xform->rotation = SbRotation(SbVec3f(0, 1, 0), diff);
						SoCylinder *cylinder = new SoCylinder;
						shape->addChild(cylinder);
						cylinder->radius = 10;
						cylinder->height = norm;
					}
				}
			}
		}
	}
	xform1->translation = SbVec3f(float(x1+aperture), float(y1+aperture), float(z1));
	return root;
}

void CFieldView::OnFieldStart3dviewer()
{
	CFragmentViewer* wnd = new CFragmentViewer;

  static LPCTSTR pMyClass = NULL;
  if (!pMyClass)
  {
    pMyClass = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, NULL, \
      NULL, AfxGetApp()->LoadIcon(IDI_3DDATA)); 
  }

	if (wnd->CreateEx(WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW, pMyClass, NULL, \
		WS_OVERLAPPEDWINDOW, 0, 0, 500, 500, AfxGetMainWnd()->GetSafeHwnd(), \
    LoadMenu(NULL, MAKEINTRESOURCE(IDR_FRAGMENT))))
	{
	  SoWinExaminerViewer *view = new SoWinExaminerViewer(wnd->GetSafeHwnd());
    wnd->view = view;

    char name[16];
    LookupWellName(GetDocument(), m_nBoreSelected, name);
    view->setTitle(name);

		SoSeparator* scene = BuildScene(GetDocument(), m_nBoreSelected, 500, 5);
	  view->setSceneGraph(scene);
	  view->show();
	  view->viewAll();

    wnd->ShowWindow(SW_SHOW);
    wnd->UpdateWindow();
	}
}

// CFragmentViewer

IMPLEMENT_DYNAMIC(CFragmentViewer, CWnd)
CFragmentViewer::CFragmentViewer(): view(0)
{
}

CFragmentViewer::~CFragmentViewer()
{
	delete view;
}

BEGIN_MESSAGE_MAP(CFragmentViewer, CWnd)
//	ON_WM_CREATE()
ON_COMMAND(ID_FILE_SAVE, OnFileSave)
ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
END_MESSAGE_MAP()

void CFragmentViewer::OnFileSave()
{
	CFileDialog dlg(FALSE, _T(".bmp"), _T("snapshot"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, \
		_T("Postscript Files (*.ps)|*.ps|") \
		_T("JPEG File Image Format (*.jpg)|*.jpg|") \
		_T("Tagged Image Format (*.tif)|*.tif|") \
		_T("Portable Network Graphics (*.png)|*.png|") \
		_T("SUN Raster Files (*.rgb)|*.rgb|") \
		_T("Windows Bitmaps (*.bmp)|*.bmp|") \
		_T("All Files (*.*)|*.*||"), \
		this);
	if (IDOK == dlg.DoModal())
	{
		SbBool ok = FALSE;
		// SbViewportRegion vp = view->getViewportRegion();
		SbViewportRegion vp(1024, 1024);
		SoOffscreenRenderer offscreen(vp);
		offscreen.setBackgroundColor(SbColor(1., 1., 1.));
		if (offscreen.render(view->getSceneManager()->getSceneGraph()))
		{
			FILE* fp = fopen(dlg.GetPathName(), "wb");
			if (fp != NULL)
			{
				if (dlg.GetFileExt() == _T("ps"))
				{
					ok = offscreen.writeToPostScript(fp);
				}
				else if (dlg.GetFileExt() == _T("jpg"))
				{
					ok = offscreen.writeToJPEG(fp);
				}
				else if (dlg.GetFileExt() == _T("tif"))
				{
					ok = offscreen.writeToTIFF(fp);
				}
				else if (dlg.GetFileExt() == _T("png"))
				{
					ok = offscreen.writeToPNG(fp);
				}
				else if (dlg.GetFileExt() == _T("rgb"))
				{
					ok = offscreen.writeToRGB(fp);
				}
				else
				{
					ok = offscreen.writeToBMP(fp);
				}
				fclose(fp);
			}
		}
		if (!ok)
		{
			AfxMessageBox(_T("Failed to write snapshot image"));
		}
	}
}

void CFragmentViewer::OnFileClose()
{
	SendMessage(WM_CLOSE, 0, 0);
}
