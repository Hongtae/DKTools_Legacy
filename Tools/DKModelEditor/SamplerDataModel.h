#include <wx/wx.h>
#include <wx/dataview.h>

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class SamplerDataModel : public wxDataViewVirtualListModel
{
public:
	struct Sampler
	{
		DKString id;
		size_t index;
		DKObject<DKTexture> texture;
	};

	enum DataViewColumn
	{
		DataViewColumnName = 0,
		DataViewColumnId,
		DataViewColumnIndex,
		DataViewColumnTarget,
		DataViewColumnFormat,
		DataViewColumnType,
		DataViewColumnWrap,
		DataViewColumnFilter,
		DataViewColumnMax,
	};

	SamplerDataModel(void)
	{
		dataViewColumns.Add(new wxDataViewColumn("Name", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnName, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Id", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnId, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Index", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnIndex, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Target", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnTarget, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Format", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnFormat, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Type", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnType, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Wrap", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnWrap, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Filter", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnFilter, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
	}
	~SamplerDataModel(void)
	{
	}
	void AddSampler(Sampler s)
	{
		wxASSERT(s.id.Length() > 0);
		wxASSERT(s.texture != NULL);

		samplers.Add(s);
		this->RowAppended();
	}
	Sampler& GetSamplerAtRow(unsigned int row)
	{
		return samplers.Value(row);
	}
	void RemoveAll(void)
	{
		samplers.Clear();
		this->Reset(0);
	}
	DKArray<wxDataViewColumn*> dataViewColumns;
protected:
	unsigned int GetColumnCount(void) const
	{
		return DataViewColumnMax;
	}
	wxString GetColumnType(unsigned int col) const
	{
		return "string";
	}
	void GetValueByRow(wxVariant &variant, unsigned int row, unsigned int col) const
	{
		const Sampler& sampler = this->samplers.Value(row);

		wxASSERT(sampler.texture != NULL);

		switch (col)
		{
		case DataViewColumnName:
			variant = (const wchar_t*)sampler.texture->Name();
			break;
		case DataViewColumnId:
			variant = (const wchar_t*)sampler.id;
			break;
		case DataViewColumnIndex:
			variant = wxString::Format("%lu", (unsigned long)sampler.index);
			break;
		case DataViewColumnTarget:
		case DataViewColumnFormat:
		case DataViewColumnType:
		case DataViewColumnWrap:
		case DataViewColumnFilter:
			variant = wxString::Format("col:%u", col);
			break;
		default:
			wxFAIL_MSG("Invalid column");
			break;
		}
	}
	bool SetValueByRow (const wxVariant &variant, unsigned int row, unsigned int col)
	{
		return false;
	}
private:
	DKArray<Sampler> samplers;
};

