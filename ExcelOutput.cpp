#include "stdafx.h"
#include "Field.h"
#include "FieldDoc.h"
#include <valarray>

#define sizeof2(type, data) sizeof(((type*)0)->data)

static const char* text_field(CFieldDoc::DictRecord* pRec)
{
  if (pRec->status.d_rus == S_OK)
  {
	  return pRec->d_rus;
  }
  else if (pRec->status.d_eng == S_OK)
  {
	  return pRec->d_eng;
  }
  else if (pRec->status.t_rus == S_OK)
  {
	  return pRec->t_rus;
  }
  else if (pRec->status.t_eng == S_OK)
  {
	  return pRec->t_eng;
  }
  else if (pRec->status.m_rus == S_OK)
  {
	  return pRec->m_rus;
  }
  else if (pRec->status.m_eng == S_OK)
  {
	  return pRec->m_eng;
  }
  return "";
}

#if 0

#include <oledb.h>

void CFieldDoc::ExcelOutput(long bore)
{
  int nq = Info(bore, 0, 0);
  if (!nq)
    return;

  std::valarray < CFieldDoc::InfoItem* > qq(nq);
  Info(bore, nq, &qq[0]);

  DBPROP dbProp;
  dbProp.dwPropertyID = DBPROP_IRowsetIndex;
  dbProp.dwOptions = DBPROPOPTIONS_REQUIRED;
  dbProp.dwStatus = DBPROPSTATUS_OK;
  dbProp.colid = DB_NULLID;
  dbProp.vValue.vt = VT_BOOL;
  dbProp.vValue.boolVal = VARIANT_TRUE;

  DBPROPSET dbPropSet;
  dbPropSet.guidPropertySet = DBPROPSET_ROWSET;
  dbPropSet.cProperties = 1;
  dbPropSet.rgProperties = &dbProp;

	struct DictRecord
	{
		long rec_item;
		long rec_group;
		char m_rus[16];
		char m_eng[16];
		char t_rus[16];
		char t_eng[16];
		char d_rus[64];
		char d_eng[64];

		struct Status
		{
			DBSTATUS m_rus;
			DBSTATUS m_eng;
			DBSTATUS t_rus;
			DBSTATUS t_eng;
			DBSTATUS d_rus;
			DBSTATUS d_eng;
		};

		Status status;
	};

	DBID TableID;
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = L"dict";

	DBID IndexID;
	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = L"group_item";

	IRowset* pRowset = 0;
	HRESULT hr = session.m_spOpenRowset->OpenRowset(NULL, &TableID, &IndexID, \
		IID_IRowset, 1, &dbPropSet, (IUnknown**) &pRowset);
	if (SUCCEEDED(hr))
	{
		IAccessor* pAccessor = 0;
		hr = pRowset->QueryInterface(IID_IAccessor, (void**) &pAccessor);
		if (SUCCEEDED(hr))
		{
			DBBINDING dbBinding[6];

			dbBinding[0].iOrdinal =    3;
			dbBinding[0].obValue =     offsetof(DictRecord, m_rus);
			dbBinding[0].obLength =    0; 
			dbBinding[0].obStatus =    offsetof(DictRecord, status.m_rus);
			dbBinding[0].pTypeInfo =   NULL;
			dbBinding[0].pObject =     NULL;
			dbBinding[0].pBindExt =    NULL;
			dbBinding[0].dwPart =      DBPART_VALUE | DBPART_STATUS;
			dbBinding[0].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
			dbBinding[0].eParamIO =    DBPARAMIO_NOTPARAM;
			dbBinding[0].cbMaxLen =    sizeof2(DictRecord, m_rus);
			dbBinding[0].dwFlags =     0;
			dbBinding[0].wType =       DBTYPE_STR;
			dbBinding[0].bPrecision =  0;
			dbBinding[0].bScale =      0;

			dbBinding[1].iOrdinal =    4;
			dbBinding[1].obValue =     offsetof(DictRecord, m_eng);
			dbBinding[1].obLength =    0; 
			dbBinding[1].obStatus =    offsetof(DictRecord, status.m_eng);
			dbBinding[1].pTypeInfo =   NULL;
			dbBinding[1].pObject =     NULL;
			dbBinding[1].pBindExt =    NULL;
			dbBinding[1].dwPart =      DBPART_VALUE | DBPART_STATUS;
			dbBinding[1].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
			dbBinding[1].eParamIO =    DBPARAMIO_NOTPARAM;
			dbBinding[1].cbMaxLen =    sizeof2(DictRecord, m_eng);
			dbBinding[1].dwFlags =     0;
			dbBinding[1].wType =       DBTYPE_STR;
			dbBinding[1].bPrecision =  0;
			dbBinding[1].bScale =      0;

			dbBinding[2].iOrdinal =    5;
			dbBinding[2].obValue =     offsetof(DictRecord, t_rus);
			dbBinding[2].obLength =    0; 
			dbBinding[2].obStatus =    offsetof(DictRecord, status.t_rus);
			dbBinding[2].pTypeInfo =   NULL;
			dbBinding[2].pObject =     NULL;
			dbBinding[2].pBindExt =    NULL;
			dbBinding[2].dwPart =      DBPART_VALUE | DBPART_STATUS;
			dbBinding[2].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
			dbBinding[2].eParamIO =    DBPARAMIO_NOTPARAM;
			dbBinding[2].cbMaxLen =    sizeof2(DictRecord, t_rus);
			dbBinding[2].dwFlags =     0;
			dbBinding[2].wType =       DBTYPE_STR;
			dbBinding[2].bPrecision =  0;
			dbBinding[2].bScale =      0;

			dbBinding[3].iOrdinal =    6;
			dbBinding[3].obValue =     offsetof(DictRecord, t_eng);
			dbBinding[3].obLength =    0; 
			dbBinding[3].obStatus =    offsetof(DictRecord, status.t_eng);
			dbBinding[3].pTypeInfo =   NULL;
			dbBinding[3].pObject =     NULL;
			dbBinding[3].pBindExt =    NULL;
			dbBinding[3].dwPart =      DBPART_VALUE | DBPART_STATUS;
			dbBinding[3].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
			dbBinding[3].eParamIO =    DBPARAMIO_NOTPARAM;
			dbBinding[3].cbMaxLen =    sizeof2(DictRecord, t_eng);
			dbBinding[3].dwFlags =     0;
			dbBinding[3].wType =       DBTYPE_STR;
			dbBinding[3].bPrecision =  0;
			dbBinding[3].bScale =      0;

			dbBinding[4].iOrdinal =    7;
			dbBinding[4].obValue =     offsetof(DictRecord, d_rus);
			dbBinding[4].obLength =    0; 
			dbBinding[4].obStatus =    offsetof(DictRecord, status.d_rus);
			dbBinding[4].pTypeInfo =   NULL;
			dbBinding[4].pObject =     NULL;
			dbBinding[4].pBindExt =    NULL;
			dbBinding[4].dwPart =      DBPART_VALUE | DBPART_STATUS;
			dbBinding[4].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
			dbBinding[4].eParamIO =    DBPARAMIO_NOTPARAM;
			dbBinding[4].cbMaxLen =    sizeof2(DictRecord, d_rus);
			dbBinding[4].dwFlags =     0;
			dbBinding[4].wType =       DBTYPE_STR;
			dbBinding[4].bPrecision =  0;
			dbBinding[4].bScale =      0;

			dbBinding[5].iOrdinal =    8;
			dbBinding[5].obValue =     offsetof(DictRecord, d_eng);
			dbBinding[5].obLength =    0; 
			dbBinding[5].obStatus =    offsetof(DictRecord, status.d_eng);
			dbBinding[5].pTypeInfo =   NULL;
			dbBinding[5].pObject =     NULL;
			dbBinding[5].pBindExt =    NULL;
			dbBinding[5].dwPart =      DBPART_VALUE | DBPART_STATUS;
			dbBinding[5].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
			dbBinding[5].eParamIO =    DBPARAMIO_NOTPARAM;
			dbBinding[5].cbMaxLen =    sizeof2(DictRecord, d_eng);
			dbBinding[5].dwFlags =     0;
			dbBinding[5].wType =       DBTYPE_STR;
			dbBinding[5].bPrecision =  0;
			dbBinding[5].bScale =      0;

			HACCESSOR hAccessor = DB_NULL_HACCESSOR;
			DBBINDSTATUS dbBindStatus[6];
			hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
				6, dbBinding, 0, &hAccessor, dbBindStatus);
			if (SUCCEEDED(hr))
			{
				IRowsetIndex *pRowsetIndex = 0;
				hr = pRowset->QueryInterface(IID_IRowsetIndex, (void**) &pRowsetIndex);
				if (SUCCEEDED(hr))
				{
					IAccessor* pIndexAccessor = 0;
					hr = pRowsetIndex->QueryInterface(IID_IAccessor, (void**) &pIndexAccessor);
					if (SUCCEEDED(hr))
					{
						dbBinding[0].iOrdinal =    2;
						dbBinding[0].obValue =     offsetof(DictRecord, rec_group);
						dbBinding[0].obLength =    0; 
						dbBinding[0].obStatus =    0;
						dbBinding[0].pTypeInfo =   NULL;
						dbBinding[0].pObject =     NULL;
						dbBinding[0].pBindExt =    NULL;
						dbBinding[0].dwPart =      DBPART_VALUE;
						dbBinding[0].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
						dbBinding[0].eParamIO =    DBPARAMIO_NOTPARAM;
						dbBinding[0].cbMaxLen =    sizeof2(DictRecord, rec_group);
						dbBinding[0].dwFlags =     0;
						dbBinding[0].wType =       DBTYPE_I4;
						dbBinding[0].bPrecision =  0;
						dbBinding[0].bScale =      0;

						dbBinding[1].iOrdinal =    1;
						dbBinding[1].obValue =     offsetof(DictRecord, rec_item);
						dbBinding[1].obLength =    0; 
						dbBinding[1].obStatus =    0;
						dbBinding[1].pTypeInfo =   NULL;
						dbBinding[1].pObject =     NULL;
						dbBinding[1].pBindExt =    NULL;
						dbBinding[1].dwPart =      DBPART_VALUE;
						dbBinding[1].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
						dbBinding[1].eParamIO =    DBPARAMIO_NOTPARAM;
						dbBinding[1].cbMaxLen =    sizeof2(DictRecord, rec_item);
						dbBinding[1].dwFlags =     0;
						dbBinding[1].wType =       DBTYPE_I4;
						dbBinding[1].bPrecision =  0;
						dbBinding[1].bScale =      0;

						HACCESSOR hIndexAccessor = DB_NULL_HACCESSOR;
						DBBINDSTATUS dbBindStatus[6];
						hr = pIndexAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 
							2, dbBinding, 0, &hIndexAccessor, dbBindStatus);
						if (SUCCEEDED(hr))
						{
							for (int i = 0; i < nq; ++i)
							{
								DictRecord dr;
								dr.rec_group = 1000101L;
								dr.rec_item = qq[i]->bed;
								hr = pRowsetIndex->Seek(hIndexAccessor, 2, &dr, DBSEEK_FIRSTEQ);
								if (S_OK == hr)
								{
									HROW hRow;
									ULONG cRowsObtained;
									HROW* pRows = &hRow;
									hr = pRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &pRows);
									if (SUCCEEDED(hr))
									{
										hr = pRowset->GetData(hRow, hAccessor, &dr);
										if (SUCCEEDED(hr))
										{
											const char* cc = 0;
											if (dr.status.d_rus == S_OK)
											{
												cc = dr.d_rus;
											}
											else if (dr.status.d_eng == S_OK)
											{
												cc = dr.d_eng;
											}
											else if (dr.status.t_rus == S_OK)
											{
												cc = dr.t_rus;
											}
											else if (dr.status.t_eng == S_OK)
											{
												cc = dr.t_eng;
											}
											else if (dr.status.m_rus == S_OK)
											{
												cc = dr.m_rus;
											}
											else if (dr.status.m_eng == S_OK)
											{
												cc = dr.m_eng;
											}
											if (cc)
											{
												fprintf(stderr, "%s\n", cc);
											}
										}
										pRowset->ReleaseRows(cRowsObtained, pRows, NULL, NULL, NULL);
									}
								}
							}
							pIndexAccessor->ReleaseAccessor(hIndexAccessor, NULL);
						}
						pIndexAccessor->Release();
					}
					pRowsetIndex->Release();
				}
				pAccessor->ReleaseAccessor(hAccessor, NULL);
			}
			pAccessor->Release();
		}
		pRowset->Release();
	}
}

#else

#import "C:\Program Files\Microsoft Office\Office11\Excel.exe" \
  auto_search auto_rename dual_interfaces exclude("IFont", "IPicture")

void CFieldDoc::ExcelOutput(long bore)
{
	int nq = Info(bore, 0, 0);
	if (!nq)
    return;

	std::valarray < CFieldDoc::InfoItem* > qq(nq);
	Info(bore, nq, &qq[0]);

  DBPROP dbProp[2];
  dbProp[0].dwPropertyID = DBPROP_IRowsetIndex;
  dbProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
  dbProp[0].dwStatus = DBPROPSTATUS_OK;
  dbProp[0].colid = DB_NULLID;
  dbProp[0].vValue.vt = VT_BOOL;
  dbProp[0].vValue.boolVal = VARIANT_TRUE;

  dbProp[1].dwPropertyID = DBPROP_IRowsetCurrentIndex;
  dbProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
  dbProp[1].dwStatus = DBPROPSTATUS_OK;
  dbProp[1].colid = DB_NULLID;
  dbProp[1].vValue.vt = VT_BOOL;
  dbProp[1].vValue.boolVal = VARIANT_TRUE;

  DBPROPSET dbPropSet;
  dbPropSet.guidPropertySet = DBPROPSET_ROWSET;
  dbPropSet.cProperties = 2;
  dbPropSet.rgProperties = dbProp;

	CComQIPtr < IRowsetIndex > spRowsetIndex;
  CTable < CAccessor < DictRecord > > dict;
	HRESULT hr = dict.Open(session, "dict", &dbPropSet);
	if (SUCCEEDED(hr))
	{
		CComQIPtr < IRowsetCurrentIndex > spRowsetCurrentIndex = dict.m_spRowset;
		if (spRowsetCurrentIndex != NULL)
		{
			DBID IndexID;
			IndexID.eKind = DBKIND_NAME;
			IndexID.uName.pwszName = L"group_item";
			hr = spRowsetCurrentIndex->SetIndex(&IndexID);
			if (SUCCEEDED(hr))
			{
				spRowsetIndex = dict.m_spRowset;
			}
		}
	}

  if (!spRowsetIndex)
    return;

  try
  {
    Excel::_ApplicationPtr app;
    HRESULT hr = app.CreateInstance("Excel.Application");
    if (SUCCEEDED(hr))
    {
      Excel::_WorkbookPtr wbk = app->Workbooks->Add((long) Excel::xlWorksheet);
      Excel::_WorksheetPtr wst = app->ActiveSheet;
      wst->Name = "geophysics";

      // When using parameterized properties, optional args must be explicitly dealt with.
      wst->Range["A1"][vtMissing]->Value2 = "геологическа€ формаци€";
      wst->Range["A1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["A1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["A1"][vtMissing]->RowHeight = 120;
      wst->Range["A1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["A1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["B1"][vtMissing]->Value2 = "литологическа€ характеристика";
      wst->Range["B1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["B1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["B1"][vtMissing]->RowHeight = 120;
      wst->Range["B1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["B1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["C1"][vtMissing]->Value2 = "характер насыщени€";
      wst->Range["C1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["C1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["C1"][vtMissing]->RowHeight = 120;
      wst->Range["C1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["C1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["D1"][vtMissing]->Value2 = "коэффициент самопроизвольной пол€ризации";
      wst->Range["D1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["D1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["D1"][vtMissing]->RowHeight = 120;
      wst->Range["D1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["D1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["E1"][vtMissing]->Value2 = "глинистость";
      wst->Range["E1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["E1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["E1"][vtMissing]->RowHeight = 120;
      wst->Range["E1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["E1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["F1"][vtMissing]->Value2 = "удельное сопротивление";
      wst->Range["F1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["F1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["F1"][vtMissing]->RowHeight = 120;
      wst->Range["F1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["F1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["G1"][vtMissing]->Value2 = "газонасыщенность по фоновому замеру";
      wst->Range["G1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["G1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["G1"][vtMissing]->RowHeight = 120;
      wst->Range["G1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["G1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["H1"][vtMissing]->Value2 = "нефтенасыщенность или газонасыщенность";
      wst->Range["H1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["H1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["H1"][vtMissing]->RowHeight = 120;
      wst->Range["H1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["H1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["I1"][vtMissing]->Value2 = "кровл€ пропластка";
      wst->Range["I1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["I1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["I1"][vtMissing]->RowHeight = 120;
      wst->Range["I1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["I1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["J1"][vtMissing]->Value2 = "подошва пропластка";
      wst->Range["J1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["J1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["J1"][vtMissing]->RowHeight = 120;
      wst->Range["J1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["J1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["K1"][vtMissing]->Value2 = "проницаемость";
      wst->Range["K1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["K1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["K1"][vtMissing]->RowHeight = 120;
      wst->Range["K1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["K1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      wst->Range["L1"][vtMissing]->Value2 = "пористость";
      wst->Range["L1"][vtMissing]->WrapText = VARIANT_TRUE;
      wst->Range["L1"][vtMissing]->Orientation = Excel::xlUpward;
      wst->Range["L1"][vtMissing]->RowHeight = 120;
      wst->Range["L1"][vtMissing]->VerticalAlignment = Excel::xlVAlignCenter;
      wst->Range["L1"][vtMissing]->HorizontalAlignment = Excel::xlHAlignCenter;

      float nodata = -9999.0f;
      for (int i = 0; i < nq; ++i)
      {
        char cell[16];

			  dict.rec_group = 1000101L;
        dict.rec_item = qq[i]->bed;
        sprintf(cell, "A%d", i+2);
			  if (S_OK == spRowsetIndex->Seek(dict.GetHAccessor(0), \
          2, (DictRecord*) &dict, DBSEEK_FIRSTEQ) && S_OK == dict.MoveNext())
			  {
          wst->Range[cell][vtMissing]->Value2 = text_field(&dict);
			  }

        dict.rec_group = 1000301L;
        dict.rec_item = qq[i]->lit;
        sprintf(cell, "B%d", i+2);
			  if (S_OK == spRowsetIndex->Seek(dict.GetHAccessor(0), \
          2, (DictRecord*) &dict, DBSEEK_FIRSTEQ) && S_OK == dict.MoveNext())
			  {
          wst->Range[cell][vtMissing]->Value2 = text_field(&dict);
			  }

        dict.rec_group = 1000201L;
        dict.rec_item = qq[i]->sat;
        sprintf(cell, "C%d", i+2);
			  if (S_OK == spRowsetIndex->Seek(dict.GetHAccessor(0), \
          2, (DictRecord*) &dict, DBSEEK_FIRSTEQ) && S_OK == dict.MoveNext())
			  {
          wst->Range[cell][vtMissing]->Value2 = text_field(&dict);
			  }

        if (qq[i]->a_sp != nodata)
        {
          sprintf(cell, "D%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->a_sp;
        }

        if (qq[i]->clay != nodata)
        {
          sprintf(cell, "E%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->clay;
        }

        if (qq[i]->f_res != nodata)
        {
          sprintf(cell, "F%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->f_res;
        }

        if (qq[i]->kg != nodata)
        {
          sprintf(cell, "G%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->kg;
        }

        if (qq[i]->kng != nodata)
        {
          sprintf(cell, "H%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->kng;
        }

        if (qq[i]->top != nodata)
        {
          sprintf(cell, "I%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->top;
        }

        if (qq[i]->bot != nodata)
        {
          sprintf(cell, "J%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->bot;
        }

        if (qq[i]->perm != nodata)
        {
          sprintf(cell, "K%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->perm;
        }

        if (qq[i]->poro != nodata)
        {
          sprintf(cell, "L%d", i+2);
          wst->Range[cell][vtMissing]->Value2 = qq[i]->poro;
        }
      }

      /*
      wst->Range["B2"][vtMissing]->Value2 = "Company B";
      wst->Range["C2"][vtMissing]->Value2 = "Company C";
      wst->Range["D2"][vtMissing]->Value2 = "Company D";

      // Of course, you can call a parameterized property 
      // as a method and then optional args are implicit.
      wst->GetRange("A3")->Value2 = 75.0;
      wst->GetRange("B3")->Value2 = 14.0;
      wst->GetRange("C3")->Value2 = 7.0;
      wst->GetRange("D3")->Value2 = 4.0;

      Excel::RangePtr range = wst->Range["A2:D3"][vtMissing];
      Excel::_ChartPtr chart = wbk->Charts->Add();
      
      chart->ChartWizard((Excel::Range*) range, (long) Excel::xl3DPie, 7L, 
        (long) Excel::xlRows, 1L, 0L, 2L, "Market Share");
      */

      app->Visible[0] = VARIANT_TRUE;
      wbk->Saved[0] = VARIANT_TRUE;
    }
  }
  catch (_com_error& e)
  {
		fprintf(stderr, "Oops -- hit an error\n");
		fprintf(stderr, "    Code = %08lx\n", e.Error());
		fprintf(stderr, "    Code Meaning = %s\n", e.ErrorMessage());
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		fprintf(stderr, "    Source = %s\n", (const char*) bstrSource);
		fprintf(stderr, "    Description = %s\n", (const char*) bstrDescription);
  }
}

#endif