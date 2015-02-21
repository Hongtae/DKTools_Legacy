#include <wx/wx.h>
#include <wx/dataview.h>

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class MaterialDataModel : public wxDataViewModel
{
public:
	enum DataViewColumn
	{
		DataViewColumnName = 0,
		DataViewColumnId,
		DataViewColumnAddress,
		DataViewColumnType,
		DataViewColumnValue,
		DataViewColumnMax,
	};

	MaterialDataModel(void)
	{
		dataViewColumns.Add(new wxDataViewColumn("Name", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnName, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Id", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnId, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Address", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnAddress, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Type", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnType, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
		dataViewColumns.Add(new wxDataViewColumn("Value", new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT), DataViewColumnValue, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE));
	}
	~MaterialDataModel(void)
	{
	}

	void RemoveAll(void)
	{
	}
	DKArray<wxDataViewColumn*> dataViewColumns;
protected:
	void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
	{
		variant = wxString::Format("test col:%u", col);
	}
	unsigned int GetColumnCount(void) const
	{
		return DataViewColumnMax;
	}
	wxString GetColumnType(unsigned int col) const
	{
		return "string";
	}
	bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
	{
		return false;
	}
	wxDataViewItem GetParent (const wxDataViewItem &item) const
	{
		return wxDataViewItem(NULL);
	}
	bool IsContainer(const wxDataViewItem &item) const 
	{
		return false;
	}
	unsigned int GetChildren(const wxDataViewItem &item, wxDataViewItemArray &children) const 
	{
		return 0;
	}
};

