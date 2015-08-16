#include "TextEditorWindow.h"

#include <algorithm>

using namespace std;



bool SplitString(const std::string& sourceStr, const char delim, std::vector<std::string>& destVector)
{
	bool endsWithDelim = true;

	int i = 0;
	int startPos = 0;

	while ((i = sourceStr.find(delim, startPos)) != string::npos)
	{
		destVector.push_back(sourceStr.substr(startPos, i - startPos));
		startPos = i + 1;
	}

	if (startPos < sourceStr.length())
	{
		destVector.push_back(sourceStr.substr(startPos, string::npos));
		endsWithDelim = false;
	}


	if (sourceStr.empty())
	{
		return false;
	}
	else
	{
		return endsWithDelim;
	}
}



void ConvertTabsIntoSpaces(std::vector<std::string>& lines)
{
	const char tab = '\t';
	const char space = ' ';

	for_each(lines.begin(), lines.end(),
		[tab, space](string& line)
	{
		int i = 0;
		while ((i = line.find(tab, i)) != string::npos)
		{
			line.erase(i, 1);
			do
			{
				line.insert(i++, 1, space);
			} while (i % 8 != 0);
		}
	});
}




void TextEditorWindow::RegisterWindowClass(char* className, UINT classStyle, LRESULT(WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM), HINSTANCE hInst)
{
	this->wc.cbSize = sizeof(wc);
	this->wc.style = classStyle;
	this->wc.lpfnWndProc = WndProc;
	this->wc.cbClsExtra = 0;
	this->wc.cbWndExtra = 0;
	this->wc.hInstance = hInst;
	this->wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	this->wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	this->wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	this->wc.lpszMenuName = nullptr;
	this->wc.lpszClassName = className;
	this->wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		string classNameStr = className;
		string message = "Cannot register class \"" + classNameStr + "\"";

		MessageBox(nullptr, message.c_str(), "Error", MB_OK);
		return;
	}
}

void TextEditorWindow::SetMyMenuSize(const RECT& clientRect)
{
	int clientRectWidth = clientRect.right - clientRect.left;
	int clientRectHeight = clientRect.bottom - clientRect.top;

	this->myMenuRect_.top = 0 + this->margin_;
	this->myMenuRect_.bottom  = this->myMenuRect_.top + this->myMenyHeight_;
	this->myMenuRect_.left = 0 + this->margin_;
	this->myMenuRect_.right = clientRectWidth - this->margin_;
}

void TextEditorWindow::SetTextAreaBorderRect(const RECT& clientRect)
{
	int clientRectWidth = clientRect.right - clientRect.left;
	int clientRectHeight = clientRect.bottom - clientRect.top;

	this->textAreaBorderRect_.top =  this->myMenuRect_.bottom + this->margin_;
	this->textAreaBorderRect_.bottom = clientRectHeight - this->margin_;
	this->textAreaBorderRect_.left = 0 + this->margin_;
	this->textAreaBorderRect_.right = clientRectWidth - this->margin_;
}

void TextEditorWindow::SetTextAreaEditRect()
{
	this->textAreaEditRect_.top =		this->textAreaBorderRect_.top		+ this->textMargin_;
	this->textAreaEditRect_.bottom =	this->textAreaBorderRect_.bottom	- this->textMargin_;
	this->textAreaEditRect_.left =		this->textAreaBorderRect_.left		+ this->textMargin_;
	this->textAreaEditRect_.right =		this->textAreaBorderRect_.right		- this->textMargin_;
}

void TextEditorWindow::SetTextMetrics()
{
	HDC deviceContext = GetDC(hWnd);

	SelectObject(deviceContext, GetStockObject(SYSTEM_FIXED_FONT));
	GetTextMetrics(deviceContext, &this->textMetrics_);

	ReleaseDC(hWnd, deviceContext);
}



void TextEditorWindow::UpdateScrollBarControlsPosition()
{
	// standard scroll box size;
	//int iHThumb = GetSystemMetrics(SM_CXHTHUMB);
	//int iVThumb = GetSystemMetrics(SM_CYVTHUMB);

	int hsbcX = this->textAreaBorderRect_.left;
	int hsbcY = this->textAreaBorderRect_.bottom - this->scrollBoxWidth_;
	int hsbcWidth = this->textAreaBorderRect_.right - textAreaBorderRect_.left - this->scrollBoxWidth_;
	int hsbcHeight = this->scrollBoxWidth_;

	MoveWindow(this->horzSBC_, hsbcX, hsbcY, hsbcWidth, hsbcHeight, true);

	int vsbcX = this->textAreaBorderRect_.right - this->scrollBoxWidth_;
	int vsbcY = this->textAreaBorderRect_.top;
	int vsbcWidth = this->scrollBoxWidth_;
	int vsbcHeight = this->textAreaBorderRect_.bottom - textAreaBorderRect_.top - this->scrollBoxWidth_;

	MoveWindow(this->vertSBC_, vsbcX, vsbcY, vsbcWidth, vsbcHeight, TRUE);
}

void TextEditorWindow::UpdateScrollInfo()
{
	int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

	this->vertScrollInfo_.nMax = this->textLines_.size() - 1;
	this->vertScrollInfo_.nPage = (this->textAreaEditRect_.bottom - this->textAreaEditRect_.top) / yStep;

	SetScrollInfo(this->vertSBC_, SB_CTL, &this->vertScrollInfo_, TRUE);
}

void TextEditorWindow::UpdateVertScroll(int scrollIncrease)
{
	int scrollRange = this->vertScrollInfo_.nMax - this->vertScrollInfo_.nMin + 1;

	// limitations;
	scrollIncrease = min(scrollIncrease, scrollRange - (int)this->vertScrollInfo_.nPage - this->vertScrollInfo_.nPos);
	scrollIncrease = max(scrollIncrease, this->vertScrollInfo_.nMin - this->vertScrollInfo_.nPos);

	if (scrollIncrease) 
	{
		this->yCaretEditAreaPos_ -= scrollIncrease;

		int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

		UpdateWindow(this->hWnd);
		ScrollWindow(this->hWnd, 0, -yStep * scrollIncrease, &this->textAreaEditRect_, &this->textAreaEditRect_);

		this->vertScrollInfo_.nPos += scrollIncrease;
		SetScrollInfo(this->vertSBC_, SB_CTL, &this->vertScrollInfo_, TRUE);
		
		InvalidateRect(this->hWnd, &this->textAreaEditRect_, false);
		UpdateWindow(this->hWnd);
	}
}

void TextEditorWindow::UpdateHorzScroll(int scrollIncrease)
{
}



void TextEditorWindow::UpdateCaretPosition()
{
	int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

	SetCaretPos(
		this->xCaretEditAreaPos_ * this->textMetrics_.tmAveCharWidth + this->xStartWritePos_,
		this->yCaretEditAreaPos_ * yStep + this->yStartWritePos_
		);
}

void TextEditorWindow::PrintText(HDC deviceContext)
{
	int startLine = this->vertScrollInfo_.nPos;

	int sum = this->vertScrollInfo_.nPos + this->vertScrollInfo_.nPage;
	int pastEndLine = (sum < this->textLines_.size()) ? sum : this->textLines_.size();

	int i = 0;
	for (int line = startLine; line < pastEndLine; ++line)
	{
		int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

		ExtTextOut(
			deviceContext,
			this->textAreaEditRect_.left,
			this->textAreaEditRect_.top + i * yStep,
			ETO_CLIPPED,
			&this->textAreaEditRect_,
			this->textLines_[line].c_str(),
			this->textLines_[line].length(),
			NULL
			);

		++i;
	}
}



void TextEditorWindow::ShowSelectedText(const bool shouldBeColored)
{
	if (this->selTextReg_.IsRegionEmpty())
	{
		return;
	}

	/*Adjusting bounds*/
	SelectedTextRegion finalRegion = this->selTextReg_.AdjustBounds();


	COLORREF oldColor;
	if (shouldBeColored)
	{
		SelectObject(this->deviceContext_, GetStockObject(SYSTEM_FIXED_FONT));
		oldColor = SetBkColor(this->deviceContext_, RGB(102, 153, 204));
	}

	// hiding caret to remove invalid tracks ...;
	if (this->isCaretVisible_)
	{
		HideCaret(this->hWnd);
	}

	if (finalRegion.logicUp_ == finalRegion.logicDown) // if selected text belongs to single line
	{
		int xStartPos = this->textAreaEditRect_.left + this->textMetrics_.tmAveCharWidth * finalRegion.logicLeft_;
		int numberOfChars = finalRegion.logicRight_ - finalRegion.logicLeft_;

		int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

		int textLineNumber = finalRegion.logicUp_;
		int textAreaLineNumber = textLineNumber - this->vertScrollInfo_.nPos;

		string strToShow = this->textLines_[textLineNumber].substr(finalRegion.logicLeft_, numberOfChars);

		ExtTextOut(
			this->deviceContext_,
			xStartPos,
			this->textAreaEditRect_.top + textAreaLineNumber * yStep,
			ETO_CLIPPED,
			&this->textAreaEditRect_,
			strToShow.c_str(),
			strToShow.length(),
			NULL
			);
	}
	else // this->selTextReg_.logicUp_ < (!) this->selTextReg_.logicDown
	{
		int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;
		
		int xStartPos = this->textAreaEditRect_.left + this->textMetrics_.tmAveCharWidth * finalRegion.logicLeft_;
		
		int textLineNumber = finalRegion.logicUp_;
		int textAreaLineNumber = textLineNumber - this->vertScrollInfo_.nPos;
		
		string firstLineStr = this->textLines_[textLineNumber].substr(finalRegion.logicLeft_, string::npos);
		
		ExtTextOut(
			this->deviceContext_,
			xStartPos,
			this->textAreaEditRect_.top + textAreaLineNumber * yStep,
			ETO_CLIPPED,
			&this->textAreaEditRect_,
			firstLineStr.c_str(),
			firstLineStr.length(),
			NULL
			);
		
		++textLineNumber;
		++finalRegion.logicUp_;
		++textAreaLineNumber;
		xStartPos = this->textAreaEditRect_.left;

		while (finalRegion.logicUp_ < finalRegion.logicDown)
		{
			ExtTextOut(
				this->deviceContext_,
				xStartPos,
				this->textAreaEditRect_.top + textAreaLineNumber * yStep,
				ETO_CLIPPED,
				&this->textAreaEditRect_,
				this->textLines_[textLineNumber].c_str(),
				this->textLines_[textLineNumber].length(),
				NULL
				);

			++finalRegion.logicUp_;
			++textLineNumber;
			++textAreaLineNumber;
		}

		string lastLineStr = this->textLines_[textLineNumber].substr(0, finalRegion.logicRight_);
		
		ExtTextOut(
			this->deviceContext_,
			xStartPos,
			this->textAreaEditRect_.top + textAreaLineNumber * yStep,
			ETO_CLIPPED,
			&this->textAreaEditRect_,
			lastLineStr.c_str(),
			lastLineStr.length(),
			NULL
			);

	}



	if (shouldBeColored)
	{
		SetBkMode(this->deviceContext_, OPAQUE);
		SetBkColor(this->deviceContext_, oldColor);
	}

	// ... and showing it again;
	if (this->isCaretVisible_)
	{
		ShowCaret(this->hWnd);
	}
}



bool TextEditorWindow::IsCaretInTextAreaBorderRect(const int cursorXPos, const int cursorYPos) const
{
	return this->textAreaBorderRect_.left < cursorXPos
		&& cursorXPos < this->textAreaBorderRect_.right
		&& this->textAreaBorderRect_.top < cursorYPos
		&& cursorYPos < this->textAreaBorderRect_.bottom;
}

bool TextEditorWindow::IsCaretInTextAreaEditRect(const int cursorXPos, const int cursorYPos) const
{
	int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading; // cursor height;

	return this->textAreaEditRect_.left <= cursorXPos
		&& cursorXPos <= this->textAreaEditRect_.right
		&& this->textAreaEditRect_.top <= cursorYPos
		&& cursorYPos <= this->textAreaEditRect_.bottom - yStep;
}



bool TextEditorWindow::CopySelectedTextToClipboard(const SelectedTextRegion& adjustedRegion) const
{
	/*creating string of text to be copied*/
	string strToCopy = "";

	if (adjustedRegion.logicUp_ == adjustedRegion.logicDown)
	{
		int startPos = adjustedRegion.logicLeft_;
		int len = adjustedRegion.logicRight_ - adjustedRegion.logicLeft_;

		strToCopy =
			this->textLines_[adjustedRegion.logicUp_].substr(startPos, len);
	}
	else
	{
		int textlineNum = adjustedRegion.logicUp_;

		strToCopy =
			this->textLines_[textlineNum++].substr(adjustedRegion.logicLeft_, string::npos) + '\n';

		while (textlineNum < adjustedRegion.logicDown)
		{
			strToCopy += (this->textLines_[textlineNum++] + '\n');
		}

		strToCopy += this->textLines_[textlineNum].substr(0, adjustedRegion.logicRight_);
	}

	/*allocating memory for storing selected text*/
	HGLOBAL hGlobalMemory = GlobalAlloc(GHND, strToCopy.length() + 1);	if (!hGlobalMemory)
	{
		MessageBox(nullptr, "Cannot allocate memory for copying", "Error", MB_OK);
		return false;
	}
	memcpy(GlobalLock(hGlobalMemory), strToCopy.c_str(), strToCopy.length() + 1);

	GlobalUnlock(hGlobalMemory);


	/*dealing with clipboard itself*/
	if (!OpenClipboard(hWnd))
	{
		MessageBox(nullptr, "Cannot open clipboard for copying", "Error", MB_OK);
		return false;
	}

	if (!EmptyClipboard())
	{
		MessageBox(nullptr, "Cannot empty clipboard for copying", "Error", MB_OK);
		return false;
	}

	if (!SetClipboardData(CF_TEXT, hGlobalMemory))
	{
		MessageBox(nullptr, "Cannot set clipboard data while copying", "Error", MB_OK);
		return false;
	}

	if (!CloseClipboard())
	{
		MessageBox(nullptr, "Cannot close clipboard while copying", "Error", MB_OK);
		return false;
	}

	return true;
}

void TextEditorWindow::RemoveSelectedTextRegion(const SelectedTextRegion& adjustedRegion)
{
	this->xCaretTextPosition_ = adjustedRegion.logicLeft_;
	this->yCaretTextPosition_ = adjustedRegion.logicUp_;

	this->xCaretEditAreaPos_ = adjustedRegion.logicLeft_;

	int diff = adjustedRegion.logicDown - adjustedRegion.logicUp_;

	this->yCaretEditAreaPos_ -=
		(this->selTextReg_.logicUp_ < this->selTextReg_.logicDown) ? diff : 0;

	this->selTextReg_
		.SetRegionEmpty(this->xCaretTextPosition_, this->yCaretTextPosition_);

	if (adjustedRegion.logicUp_ == adjustedRegion.logicDown)
	{
		int startPos = adjustedRegion.logicLeft_;
		int len = adjustedRegion.logicRight_ - adjustedRegion.logicLeft_;

		this->textLines_[adjustedRegion.logicUp_].erase(startPos, len);
	}
	else
	{
		int startPos = adjustedRegion.logicRight_;

		string endOfLastLine =
			this->textLines_[adjustedRegion.logicDown].substr(startPos, string::npos);

		int len = adjustedRegion.logicLeft_;

		this->textLines_[adjustedRegion.logicUp_] =
			this->textLines_[adjustedRegion.logicUp_].substr(0, len) + endOfLastLine;

		auto begIter = this->textLines_.begin();
		auto endIter = this->textLines_.begin();
		int i = 0;

		while (i++ <= adjustedRegion.logicUp_)
		{
			++begIter;
			++endIter;
		}

		while (i++ <= adjustedRegion.logicDown)
		{
			++endIter;
		}

		this->textLines_.erase(begIter, ++endIter);

		this->UpdateScrollInfo();
	}

	this->AdjustVertScrollingToCaretLine();
	this->UpdateCaretPosition();

	InvalidateRect(hWnd, &this->textAreaEditRect_, TRUE);
}

void TextEditorWindow::AdjustVertScrollingToCaretLine()
{
	int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

	int xCursorPos = this->xCaretEditAreaPos_ * this->textMetrics_.tmAveCharWidth + this->xStartWritePos_;
	int yCursorPos = this->yCaretEditAreaPos_ * yStep + this->yStartWritePos_;

	if (!this->IsCaretInTextAreaEditRect(xCursorPos, yCursorPos))
	{
		int scrollIncrease = 0;

		if (this->yCaretEditAreaPos_ < 0)
		{
			scrollIncrease = this->yCaretEditAreaPos_;
		}
		else
		{
			scrollIncrease = this->yCaretEditAreaPos_ - this->vertScrollInfo_.nPage + 1;
		}

		SendMessage(hWnd, WM_VSCROLL, MAKELONG(SB_THUMBTRACK, this->vertScrollInfo_.nPos + scrollIncrease), NULL);
	}
}



TextEditorWindow::TextEditorWindow(
		LPCSTR windowName,
		HINSTANCE hInst,
		int cmdShow,
		LRESULT(WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM),
		LPCSTR menuName,
		int x,
		int y,
		int width,
		int height,
		UINT classStyle,
		DWORD windowStyle,
		HWND hParent
	):
	margin_(10),
	textMargin_(10),
	myMenyHeight_(40),
	scrollBoxWidth_(10),
	isCaretVisible_(true),
	deviceContext_(nullptr)
{
	char szClassName[] = "KWndClass";

	this->RegisterWindowClass(szClassName, classStyle, WndProc, hInst);


	hWnd = CreateWindow(szClassName, windowName, windowStyle, x, y, width, height, hParent, (HMENU)nullptr, hInst, nullptr);

	if (!hWnd)
	{
		string windowName = windowName;
		string message = "Cannot create window \"" + windowName + "\"";

		MessageBox(nullptr, message.c_str(), "Error", MB_OK);
		return;
	}

	// attaching window to concrete text editor instance;
	hwndTextEditorWndMap[hWnd] = this;

	/* new */
	//this->RegisterHotKeys();
	/**/

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	this->SetTextMetrics();
	this->SetMyMenuSize(clientRect);
	this->SetTextAreaBorderRect(clientRect);
	this->SetTextAreaEditRect();

	/* in final version must be: */
	//this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");

	/* hardcoded for testing: */
	this->textLines_.push_back("1. Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("2. And what about  you?");
	this->textLines_.push_back("3. London is a capital of Great Britain");
	this->textLines_.push_back("4. No comments");
	this->textLines_.push_back("5. if(a > b && c < d) a = b;");
	this->textLines_.push_back("");
	this->textLines_.push_back("The earliest written record of a windmill is from Yorkshire, England, dated 1185.");
	this->textLines_.push_back("Paper manufacture began in Italy around 1270.");
	this->textLines_.push_back("The spinning wheel was brought to Europe (probably from India) in the 13th century.");
	this->textLines_.push_back("The magnetic compass aided navigation, first reaching Europe some time in the late 12th century.");
	this->textLines_.push_back("Eyeglasses were invented in Italy in the late 1280s.");
	this->textLines_.push_back("The astrolabe returned to Europe via Islamic Spain.");
	this->textLines_.push_back("Leonardo of Pisa introduces Arabic numerals to Europe with his book Liber Abaci in 1202.");
	this->textLines_.push_back("The West's oldest known depiction of a stern-mounted rudder can be found on church carvings dating to around 1180.");
	this->textLines_.push_back("Art in the High Middle Ages includes these important movements:");
	this->textLines_.push_back("Anglo-Saxon art was influential on the British Isles until the Norman Invasion of 1066");
	this->textLines_.push_back("Romanesque art continued traditions from the Classical world (not to be confused with Romanesque architecture)");
	this->textLines_.push_back("Gothic art developed a distinct Germanic flavor(not to be confused with Gothic architecture).");
	this->textLines_.push_back("Byzantine art continued earlier Byzantine traditions, influencing much of Eastern Europe.");
	this->textLines_.push_back("Illuminated manuscripts gained prominence both in the Catholic and Orthodox churches");
	this->textLines_.push_back("Dates:");
	this->textLines_.push_back("1003 — death of Pope Sylvester II");
	this->textLines_.push_back("1018 — the First Bulgarian Empire is conquered by the Byzantine Empire under Basil II.");
	this->textLines_.push_back("1027 — the Salian Conrad II succeeds the last Ottonian Henry II the Saint");
	this->textLines_.push_back("1054 — East-West Schism");
	this->textLines_.push_back("1066 — Battle of Hastings");
	this->textLines_.push_back("1066 – 1067 Bayeux Tapestry");
	this->textLines_.push_back("1073 – 1085 — Pope Gregory VII");
	this->textLines_.push_back("1071 — Battle of Manzikert");
	this->textLines_.push_back("1077 — Henry IV's Walk to Canossa");
	this->textLines_.push_back("1086 — Domesday Book");
	this->textLines_.push_back("1088 — University of Bologna founded");
	

	this->xStartWritePos_ = this->textAreaEditRect_.left;
	this->yStartWritePos_ = this->textAreaEditRect_.top;

	this->xCaretEditAreaPos_ = 0;
	this->yCaretEditAreaPos_ = 0;

	//this->isCaretVisible_ = true;

	this->xCaretTextPosition_ = 0;
	this->yCaretTextPosition_ = 0;

	/* creating scroll bar controls*/

	int hsbcX = this->textAreaBorderRect_.left;
	int hsbcY = this->textAreaBorderRect_.bottom - this->scrollBoxWidth_;
	int hsbcWidth = this->textAreaBorderRect_.right - textAreaBorderRect_.left - this->scrollBoxWidth_;
	int hsbcHeight = this->scrollBoxWidth_;

	// Create a horizontal scroll bar to put in the window.
	this->horzSBC_ = CreateWindow(
			"SCROLLBAR",
			(LPSTR)NULL,
			WS_CHILD | WS_VISIBLE | SBS_HORZ,
			hsbcX, hsbcY, hsbcWidth, hsbcHeight,
			this->hWnd,
			(HMENU)NULL,
			(HINSTANCE)hInst,
			NULL
		);

	int vsbcX = this->textAreaBorderRect_.right - this->scrollBoxWidth_;
	int vsbcY = this->textAreaBorderRect_.top;
	int vsbcWidth = this->scrollBoxWidth_;
	int vsbcHeight = this->textAreaBorderRect_.bottom - textAreaBorderRect_.top - this->scrollBoxWidth_;

	// Create a vertical scroll bar to put in the window.
	this->vertSBC_ = CreateWindow(
			"Scrollbar",
			(LPSTR)NULL,
			WS_CHILD | WS_VISIBLE | SBS_VERT | SBS_RIGHTALIGN,
			vsbcX, vsbcY, vsbcWidth, vsbcHeight,
			this->hWnd,
			(HMENU)NULL,
			hInst,
			NULL
		);

	int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

	this->vertScrollInfo_.cbSize = sizeof(SCROLLINFO);
	this->vertScrollInfo_.nMin = 0;
	this->vertScrollInfo_.nMax = this->textLines_.size() - 1;
	this->vertScrollInfo_.nPage = (this->textAreaEditRect_.bottom - this->textAreaEditRect_.top) / yStep/* - 1*/;
	this->vertScrollInfo_.nPos = this->vertScrollInfo_.nMin;
	this->vertScrollInfo_.fMask = SIF_ALL;//SIF_RANGE | SIF_PAGE | SIF_POS;

	SetScrollInfo(this->vertSBC_, SB_CTL, &this->vertScrollInfo_, TRUE);
	
	// text selection;
	bool isMouseTracked_ = false;

	ShowWindow(this->hWnd, cmdShow);
}


HWND TextEditorWindow::GetHWnd()
{
	return hWnd;
}


TextEditorWindow::~TextEditorWindow()
{
	//EndPaint(hWnd, &this->ps_);
}


map<HWND, TextEditorWindow*> TextEditorWindow::hwndTextEditorWndMap;