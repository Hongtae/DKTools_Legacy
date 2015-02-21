#include "StdAfx.h"
#include <stack>
#include "ShaderEditor.h"

void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
DKMaterial* GetMaterial(void);
void SetMaterialModified(void);


wxBEGIN_EVENT_TABLE(ShaderEditor, wxAuiNotebook)
	EVT_STC_CHANGE(wxID_ANY,				ShaderEditor::OnChange)
	EVT_STC_CHARADDED(wxID_ANY,				ShaderEditor::OnCharAdded)
    EVT_STC_MARGINCLICK (wxID_ANY,			ShaderEditor::OnMarginClick)
	EVT_STC_UPDATEUI(wxID_ANY,				ShaderEditor::OnUpdateUI)
	EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY,	ShaderEditor::OnPageClose)
wxEND_EVENT_TABLE();

inline ShaderEditor::Block MakeBlock(const wchar_t* begin, const wchar_t* end, wxColour color)
{
	ShaderEditor::Block	b = {begin, end, color};
	return b;
}

ShaderEditor::ShaderEditor(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxAuiNotebook(parent, id, pos, size, style)
	, lineNoMarker(0)
	, folderMarker(1)
{

}

ShaderEditor::~ShaderEditor(void)
{
}

int ShaderEditor::FindPage(DKMaterial::ShaderSource* shader) const
{
	wxASSERT(shader);

	for (int i = 0; i < GetPageCount(); i++)
	{
		if (reinterpret_cast<PageData*>(GetPage(i)->GetClientObject())->shader == shader)
		{
			return i;
		}
	}
	return -1;
}

const char* keywords =
"attribute const uniform varying layout centroid flat smooth noperspective break continue do for while "
"switch case default if else in out inout float int void bool true false invariant discard return "
"mat2 mat3 mat4 mat2x2 mat2x3 mat2x4 mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 "
"vec2 vec3 vec4 ivec2 ivec3 ivec4 bvec2 bvec3 bvec4 uint uvec2 uvec3 uvec4 lowp mediump highp precision "
"sampler1D sampler2D sampler3D samplerCube sampler1DShadow sampler2DShadow samplerCubeShadow "
"sampler1DArray sampler2DArray sampler1DArrayShadow sampler2DArrayShadow isampler1D isampler2D isampler3D "
"isamplerCube isampler1DArray isampler2DArray usampler1D usampler2D usampler3D usamplerCube "
"usampler1DArray usampler2DArray sampler2DRect sampler2DRectShadow isampler2DRect usampler2DRect "
"samplerBuffer isamplerBuffer usamplerBuffer sampler2DMS isampler2DMS usampler2DMS "
"sampler2DMSArray isampler2DMSArray usampler2DMSArray struct "		// default
"common partition active asm class union enum typedef template this packed goto "
"inline noinline volatile public static extern external interface long short double half fixed unsigned superp "
"input output hvec2 hvec3 hvec4 dvec2 dvec3 dvec4 fvec2 fvec3 fvec4 sampler3DRect filter "
"image1D image2D image3D imageCube iimage1D iimage2D iimage3D iimageCube uimage1D uimage2D uimage3D uimageCube "
"image1DArray image2DArray iimage1DArray iimage2DArray uimage1DArray uimage2DArray image1DShadow image2DShadow "
"image1DArrayShadow image2DArrayShadow imageBuffer iimageBuffer uimageBuffer sizeof cast namespace using row_major"; // reserved
const char* builtinFunctions =
"radians degrees sin cos tan asin acos atan sinh cosh tanh asinh acosh atanh "	// angle & trigonometry
"pow exp log exp2 log2 sqrt inversesqrt "	// exponential
"abs sign floor trunc round roundEven ceil fract mod modf min max clamp mix step smoothstep isnan isinf "	// common
"length distance dot cross normalize ftransform faceforward reflect refract "	// geometric
"matrixCompMulti outerProduct transpose determinant inverse "		// matrix
"lessThan lessThanEqual greaterThan greaterThanEqual equal notEqual any all not "	// vector
"dFdx dFdy fwidth "		// derivative
"noise1 noise2 noise3 noise4 "		// noise
"EmitVertex EndPrimitive "			// geometry shader
"textureSize texture textureProj textureLod textureOffset texelFetch texelFetchOffset "
"textureProjOffset textureLodOffset textureProjLod textureProjLodOffset textureGrad "
"textureGradOffset textureProjGrad textureProjGradOffset";		// texture lookup

void ShaderEditor::Open(DKMaterial::ShaderSource* shader)
{
	wxASSERT(shader);

	// 편집창 열고 활성화
	int page = FindPage(shader);
	if (page >= 0)
	{
		SetSelection(page);
		return;
	}

	wxStyledTextCtrl *p = new wxStyledTextCtrl(this, wxID_ANY);
	p->SetClientObject(new PageData(shader));

	wxFont defaultFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	// lexer
	p->StyleClearAll();
	p->SetLexer(wxSTC_LEX_CPP);

	for (int i = 0; i < wxSTC_STYLE_LASTPREDEFINED; i++)
	{
		p->StyleSetFont(i, defaultFont);
	}

	////////////////////////////////////////////////////////////////////////////////
	SetInternalStyle(p, wxSTC_STYLE_DEFAULT,			defaultFont, "BLACK",				"WHITE");
	SetInternalStyle(p, wxSTC_STYLE_LINENUMBER,			defaultFont, "DARK GREY",			"WHITE");
	SetInternalStyle(p, wxSTC_STYLE_BRACELIGHT,			defaultFont, "BLUE", 				"WHITE", true);
	SetInternalStyle(p, wxSTC_STYLE_BRACEBAD,			defaultFont, "RED", 				"WHITE", true);
	SetInternalStyle(p, wxSTC_STYLE_CONTROLCHAR,		defaultFont, "RED", 				"DARK GREY");
	SetInternalStyle(p, wxSTC_STYLE_INDENTGUIDE,		defaultFont, "LIGHT GREY",			"WHITE");
	SetInternalStyle(p, wxSTC_STYLE_CALLTIP,			defaultFont, "DARK GREY",			"RED");
	////////////////////////////////////////////////////////////////////////////////
	SetInternalStyle(p, wxSTC_C_DEFAULT,				defaultFont, "BLACK", 				"WHITE");	// 기본 스타일
	SetInternalStyle(p, wxSTC_C_COMMENT,				defaultFont, wxColour(0,128,0), 	"WHITE");	// 일반 주석
	SetInternalStyle(p, wxSTC_C_COMMENTLINE,			defaultFont, wxColour(0,128,0), 	"WHITE");	// 일반 주석
	SetInternalStyle(p, wxSTC_C_COMMENTDOC,				defaultFont, wxColour(0,128,0), 	"WHITE");
	SetInternalStyle(p, wxSTC_C_NUMBER,					defaultFont, "RED", 				"WHITE");
	SetInternalStyle(p, wxSTC_C_WORD,					defaultFont, "BLUE",				"WHITE");
	SetInternalStyle(p, wxSTC_C_STRING,					defaultFont, "RED", 				"WHITE");
	SetInternalStyle(p, wxSTC_C_CHARACTER,				defaultFont, "RED", 				"WHITE");
	SetInternalStyle(p, wxSTC_C_UUID,					defaultFont, "PALE GREEN", 			"WHITE");
	SetInternalStyle(p, wxSTC_C_PREPROCESSOR,			defaultFont, "BLUE", 				"WHITE");
	SetInternalStyle(p, wxSTC_C_OPERATOR,				defaultFont, "DIM GREY", 			"WHITE");
	SetInternalStyle(p, wxSTC_C_IDENTIFIER,				defaultFont, "BLACK", 				"WHITE");		// 일반 문자 (언어 기본)
	SetInternalStyle(p, wxSTC_C_STRINGEOL,				defaultFont, "CADET BLUE",			"WHITE");
	SetInternalStyle(p, wxSTC_C_VERBATIM,				defaultFont, "DARK GREY",			"WHITE");
	SetInternalStyle(p, wxSTC_C_REGEX,					defaultFont, "YELLOW GREEN", 		"WHITE");
	SetInternalStyle(p, wxSTC_C_COMMENTLINEDOC,			defaultFont, "LIME GREEN", 			"WHITE");
	SetInternalStyle(p, wxSTC_C_WORD2,					defaultFont, "RED",					"WHITE");
	SetInternalStyle(p, wxSTC_C_COMMENTDOCKEYWORD,		defaultFont, "MEDIUM BLUE",			"WHITE");
	SetInternalStyle(p, wxSTC_C_COMMENTDOCKEYWORDERROR,	defaultFont, "LIGHT MAGENTA", 		"WHITE");
	SetInternalStyle(p, wxSTC_C_GLOBALCLASS,			defaultFont, "MAGENTA", 			"WHITE");
	////////////////////////////////////////////////////////////////////////////////

	p->SetKeyWords(0, keywords);
	p->SetKeyWords(1, builtinFunctions);
	p->SetKeyWords(2, "");
	p->SetKeyWords(4, "");

	////////////////////////////////////////////////////////////////////////////////

	// set visibility
	p->SetVisiblePolicy(wxSTC_VISIBLE_STRICT|wxSTC_VISIBLE_SLOP, 1);
	p->SetXCaretPolicy(wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);
	p->SetYCaretPolicy(wxSTC_CARET_EVEN|wxSTC_VISIBLE_STRICT|wxSTC_CARET_SLOP, 1);

    // markers
	p->MarkerDefine(wxSTC_MARKNUM_FOLDER,			wxSTC_MARK_BOXPLUS,				"WHITE", "BLACK");			// 바깥 접힌 블록
	p->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN,		wxSTC_MARK_BOXMINUS,			"WHITE", "BLACK");			// 바깥 열린 블록
	p->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB,		wxSTC_MARK_VLINE,				"WHITE", "BLACK");			// 중간 내용
	p->MarkerDefine(wxSTC_MARKNUM_FOLDEREND,		wxSTC_MARK_BOXPLUSCONNECTED,	"WHITE", "BLACK");			// 안쪽 접힌 블록
	p->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID,	wxSTC_MARK_BOXMINUSCONNECTED,	"WHITE", "BLACK");			// 안쪽 열린 블록
	p->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL,	wxSTC_MARK_TCORNER,				"WHITE", "BLACK");			// 안쪽 블록 끝
	p->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL,		wxSTC_MARK_LCORNER,				"WHITE", "BLACK");			// 바깥쪽 블록 끝

	// margin
	// 0: display line numbers
	// 1: non-folding symbols (16-width)
	// 2: folding symbols

	// 라인넘버 두께 설정 (OnChange 에서 다시 설정한다)
	p->SetMarginType(lineNoMarker, wxSTC_MARGIN_NUMBER);
	p->SetMarginWidth(lineNoMarker, p->TextWidth(wxSTC_STYLE_LINENUMBER, "88"));

	// folding
	p->SetMarginType(folderMarker, wxSTC_MARGIN_SYMBOL);
	p->SetMarginMask(folderMarker, wxSTC_MASK_FOLDERS);
	p->SetMarginWidth(folderMarker, 16);
	p->SetMarginSensitive(folderMarker, true);

	p->SetProperty("fold", "1");
	p->SetProperty("fold.comment", "1");
	p->SetProperty("fold.compact", "1");
	p->SetProperty("fold.preprocessor", "1");
	p->SetProperty("styling.within.preprocessor", "1");
	
	p->SetFoldFlags(
	//	wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED |		// 열린 블록 위에 선 그림
		wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |		// 접힌 블록 위에 선 그림
	//	wxSTC_FOLDFLAG_LINEAFTER_EXPANDED |			// 열린 블록 아래에 선 그림
		wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED |		// 접힌 블록 아래에 선 그림
	//	wxSTC_FOLDFLAG_LEVELNUMBERS |				// 폴딩 정보를 라인 넘버 위치에 표시 (디버깅)
	//	wxSTC_FOLDFLAG_BOX |						// 폴딩 박스 그리기	
		0);

	// set spaces and indention
	p->SetTabWidth(4);
	p->SetUseTabs(true);
	p->SetTabIndents(true);
	p->SetBackSpaceUnIndents(true);
	p->SetIndent(4);
	p->SetIndentationGuides(wxSTC_IV_REAL);

	p->SetModEventMask(wxSTC_MOD_INSERTTEXT | wxSTC_MOD_DELETETEXT | wxSTC_PERFORMED_USER | wxSTC_PERFORMED_UNDO  | wxSTC_PERFORMED_REDO);

	AddPage(p, (const wchar_t*)shader->name, true);

	Update(shader);
}

void ShaderEditor::SetInternalStyle(wxStyledTextCtrl* ctrl, int style, wxFont& fnt, const wxColour& fc, const wxColour& bc, bool bold, bool italic, bool underline, bool visible, int caseforce)
{
	ctrl->StyleSetFont(style, fnt);	
	ctrl->StyleSetForeground(style, fc);
	ctrl->StyleSetBackground(style, bc);
	ctrl->StyleSetBold(style, bold);
	ctrl->StyleSetItalic(style, italic);
	ctrl->StyleSetUnderline(style, underline);
	ctrl->StyleSetVisible(style, visible);
	ctrl->StyleSetCase(style, caseforce);
}

void ShaderEditor::Update(DKMaterial::ShaderSource* shader)
{
	int page = FindPage(shader);
	if (page >= 0)
	{
		SetPageText(page, (const wchar_t*)shader->name);
		wxStyledTextCtrl* p = (wxStyledTextCtrl*)GetPage(page);
		p->SetText((const wchar_t*)shader->source);
	}
}

bool ShaderEditor::Close(DKMaterial::ShaderSource* shader)
{
	int page = FindPage(shader);
	if (page >= 0)
	{
		DeletePage(page);
	}
	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, (const wchar_t*)shader->name), wxColour(255,0,0));
	return true;
}

void ShaderEditor::CloseAll(void)
{
	while (DeletePage(0));
}

void ShaderEditor::OnChange(wxStyledTextEvent &e)
{
	wxStyledTextCtrl* ctrl = (wxStyledTextCtrl*)e.GetEventObject();
	PageData* data = (PageData*)ctrl->GetClientObject();
	wxASSERT(data);
	data->shader->source = (const wchar_t*)ctrl->GetText();

	wxString tmp = "8";		// 표시할 라인 너비를 구하기 위한 스트링
	for (int lines = ctrl->GetLineCount(); lines > 0; lines /= 10)		tmp += "8";

	ctrl->SetMarginWidth(lineNoMarker, ctrl->TextWidth(wxSTC_STYLE_LINENUMBER, tmp));

//	PrintLog(wxString::Format("[%s] in shader(\"%ls\")\n", DKLIB_FUNCTION_NAME, data->shader->name), wxColour(255,0,0));
	SetMaterialModified();		// 값이 수정됨
}

inline bool IsWhitespace(wchar_t c)
{
	if (c == L' ' || c == L'\t' || c == L'\r' || c == L'\n')
		return true;
	return false;
}

bool IsStringAllWhiteSpace(const wchar_t* str, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (!IsWhitespace(str[i]))
			return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// 엔터키
//	1. 위 라인이 문장 시작 라인이며, 종결되었다면 위 라인과 같은 인덴트
//	2. 위 라인이 문장 시작 라인이며, 종결되지 않았다면 위 라인 인덴트 + 1
//	3. 위 라인이 문장 시작 라인이 아니면, 위 라인과 같은 인덴트
//		문장 종결은 세미콜론으로 끝나야 함
//		{ 은 문장 시작라인으로 간주
//	4. 위 라인이 주석 시작 라인이면 주석 시작지점이 인덴트
//	5. 위 라인이 주석 라인이며 시작 라인이 아닐경우 위와 같은 인덴트
//	6. 위 라인이 종결되지 않은 스트링에 \ 부호로 끝나면 스트링 시작위치가 다음라인 시작지점
////////////////////////////////////////////////////////////////////////////////
// { 키
//	1. 마지막 문장 시작 라인의 인덴트와 같은 인덴트
////////////////////////////////////////////////////////////////////////////////
// } 키
//	1. 가장 가까운 { 를 찾은 후 그 위의 마지막 문장 시작라인과 같은 인덴트
////////////////////////////////////////////////////////////////////////////////
// : 키
//	1. 가장 가까운 문장 시작라인과 같은 인덴트
////////////////////////////////////////////////////////////////////////////////
// 윗줄의 인덴트가 0 이면 무조건 0 임. (주석은 주석 시작위치)
// # 으로 시작하면 인덴트 무조건 0
// 

// 아래 객체를 스택에 담아서 쓴다 가장 마지막이 현재 블록
struct IndentState
{
	bool statementBlock;
	bool commentBlock;
	bool stringBlock;
	int	statementIndent;			// 가장 마지막 시작 문장 인덴트	(완성된 문장의 인덴트, 노드참조용, 블록시작시 참조)
	int nextIndent;					// 가장 마지막 라인 인덴트		(일반 문장에서 참조)
	int previousIndent;				// 상위 블록의 statementIndent	(이전 블록의 인덴트, 블록 종료시 참조)
};

int increaseLineIndent(const int p, const int indentSize)
{
	int i = 0;
	for (i = 0; i <= p; i += indentSize);
	return i;
}

void BuildIndentStack(const wchar_t* line, size_t len, int indent, int indentSize, std::stack<IndentState>& indentStack)
{
	int i = 0;
	for (i = 0; i < len; i++)
	{
		if (!IsWhitespace(line[i]))
			break;
	}
	if (line[i] == L'#')
		return;

	bool escapeSeq = false;
	bool quote = false;

	if (!indentStack.top().statementBlock)
		indentStack.top().statementIndent = indent;

	for (; i < len; i++)
	{
		if (escapeSeq)
		{
			escapeSeq = false;
			continue;
		}

		IndentState& state = indentStack.top();

		if (state.commentBlock)
		{
			if (line[i] == L'*' && line[i+1] == L'/')
				state.commentBlock = false;
		}
		else if (state.stringBlock)
		{
			if (!escapeSeq && line[i] == L'\"')
				state.stringBlock = false;
		}
		else if (line[i] == L'/')
		{
			if (line[i+1] == L'*')				// 주석 블럭 시작
			{
				state.commentBlock = true;
				state.nextIndent = i;
			}
			else if (line[i+1] == L'/')			// 주석 라인 시작, 리턴
				return;
		}
		else if (line[i] == L'\"')
		{
			if (!escapeSeq)
			{
				state.stringBlock = true;
				state.nextIndent = i;
			}
		}
		else if (line[i] == L'\'')
		{
			quote = !quote;
		}
		else if (line[i] == L'{')
		{
			state.statementBlock = false;
			state.nextIndent = state.statementIndent;

			IndentState child;
			memset(&child, 0, sizeof(IndentState));
			child.statementBlock = false;
			child.statementIndent = increaseLineIndent(indent, indentSize);
			child.nextIndent = child.statementIndent;
			child.previousIndent = state.statementIndent;
			indentStack.push(child);
		}
		else if (line[i] == L'}')
		{
			if (indentStack.size() > 1)
				indentStack.pop();
			else
			{
				indentStack.top().statementIndent = 0;
				indentStack.top().nextIndent = 0;
				indentStack.top().previousIndent = 0;
			}
			indentStack.top().statementBlock = false;
			indentStack.top().nextIndent = indentStack.top().statementIndent;
		}
		else if (line[i] == L';')
		{
			state.statementBlock = false;
			state.nextIndent = state.statementIndent;
		}
		else if (line[i] == L':')
		{
			state.statementBlock = false;
			state.nextIndent = increaseLineIndent(state.statementIndent, indentSize);
		}
		else if (!IsWhitespace(line[i]))
		{
			state.statementBlock = true;
			state.nextIndent = increaseLineIndent(state.statementIndent, indentSize);
		}

		if (line[i] == L'\\')		// escape-sequence 도 일반 statement 로 처리하기 위해서 맨 마지막에 있어야 한다.
			escapeSeq = true;
	}
}

void GetStatementIndent(wxStyledTextCtrl* ctrl, IndentState& state)
{
	int indent = ctrl->GetIndent();
	int currentLine = ctrl->GetCurrentLine();
	memset(&state, 0, sizeof(IndentState));

	std::stack<IndentState>	indentStack;
	indentStack.push(state);
	for (int i = 0; i < currentLine; i++)
	{
		BuildIndentStack(ctrl->GetLine(i), ctrl->GetLineLength(i), ctrl->GetLineIndentation(i), indent, indentStack);
	}
	state = indentStack.top();

	DKLog("--------------------------------------------------\n");
	int max = indentStack.size();
	for (int i = 0; !indentStack.empty(); i++)
	{
		IndentState& state = indentStack.top();
		DKLog("indent[%d/%d] state:%d, comment:%d, string:%d, sIndent:%d, nIndent:%d, pIndent:%d\n",
			i, max,
			state.statementBlock,
			state.commentBlock,
			state.stringBlock,
			state.statementIndent,
			state.nextIndent,
			state.previousIndent);

		indentStack.pop();
	}
}

void ShaderEditor::OnCharAdded(wxStyledTextEvent &e)
{
	wxStyledTextCtrl* ctrl = (wxStyledTextCtrl*)e.GetEventObject();
	char c = (char)e.GetKey();
	int indent = ctrl->GetIndent();
	int currentLine = ctrl->GetCurrentLine();

	if (c == '\n')
	{
		IndentState s;
		GetStatementIndent(ctrl, s);
		ctrl->SetLineIndentation(currentLine, s.nextIndent);
		int p = ctrl->GetLineIndentPosition(currentLine);
		ctrl->GotoPos(p);
	}
	else if (c == ':')
	{
		IndentState s;
		GetStatementIndent(ctrl, s);
		ctrl->SetLineIndentation(currentLine, s.previousIndent);
	}
	else if (c == '{')
	{
		if (IsStringAllWhiteSpace(ctrl->GetLine(currentLine), ctrl->GetCurrentPos() - ctrl->PositionFromLine(currentLine) - 1))
		{
			IndentState s;
			GetStatementIndent(ctrl, s);
			ctrl->SetLineIndentation(currentLine, s.statementIndent);
		}
	}
	else if (c == '}')
	{
		if (IsStringAllWhiteSpace(ctrl->GetLine(currentLine), ctrl->GetCurrentPos() - ctrl->PositionFromLine(currentLine) - 1))
		{
			IndentState s;
			GetStatementIndent(ctrl, s);
			ctrl->SetLineIndentation(currentLine, s.previousIndent);
		}
	}
	else if (c == '#')		// preprocessor
	{
		if (IsStringAllWhiteSpace(ctrl->GetLine(currentLine), ctrl->GetCurrentPos() - ctrl->PositionFromLine(currentLine) - 1))
		{
			ctrl->SetLineIndentation(currentLine, 0);
		}
	}
}

void ShaderEditor::OnUpdateUI(wxStyledTextEvent &e)
{
	wxStyledTextCtrl* ctrl = (wxStyledTextCtrl*)e.GetEventObject();
	int current = ctrl->GetCurrentPos();

	char c = ctrl->GetCharAt(current);
	int style = ctrl->GetStyleAt(current);
	if ((c == '(' || c == '[' || c == '{' || c == '<' || c == ')' || c == ']' || c == '}' || c == '>') && style == wxSTC_C_OPERATOR)
	{
		int pos = ctrl->BraceMatch(current);
		if (pos != wxSTC_INVALID_POSITION)
			ctrl->BraceHighlight(current, pos);
		else
			ctrl->BraceBadLight(current);
	}
	else
	{
		char c = ctrl->GetCharAt(current-1);
		int style = ctrl->GetStyleAt(current-1);
		if ((c == '(' || c == '[' || c == '{' || c == '<' || c == ')' || c == ']' || c == '}' || c == '>') && style == wxSTC_C_OPERATOR)
		{
			int pos = ctrl->BraceMatch(current-1);
			if (pos != wxSTC_INVALID_POSITION)
				ctrl->BraceHighlight(current-1, pos);
			else
				ctrl->BraceBadLight(current-1);
		}
		else
			ctrl->BraceBadLight(-1);
	}
}

void ShaderEditor::OnPageClose(wxAuiNotebookEvent& e)
{
	PageData* data = (PageData*)GetPage(e.GetSelection())->GetClientObject();
	wxASSERT(data);
	PrintLog(wxString::Format("[%s] in shader(\"%ls\")\n", DKLIB_FUNCTION_NAME, (const wchar_t*)data->shader->name), wxColour(255,0,0));
}

void ShaderEditor::OnMarginClick(wxStyledTextEvent &e)
{
	if (e.GetMargin() == folderMarker)
	{
		wxStyledTextCtrl* ctrl = (wxStyledTextCtrl*)e.GetEventObject();

		int lineClick = ctrl->LineFromPosition(e.GetPosition());
		int levelClick = ctrl->GetFoldLevel(lineClick);
		if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
		{
			ctrl->ToggleFold(lineClick);
		}
	}
}

