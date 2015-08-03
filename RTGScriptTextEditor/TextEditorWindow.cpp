#include "TextEditorWindow.h"

using namespace std;



void TextEditorWindow::RegisterWindowClass(char* className, UINT classStyle, LRESULT(WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM), HINSTANCE hInst)
{
	this->wc.cbSize = sizeof(wc);
	this->wc.style = classStyle;
	this->wc.lpfnWndProc = WndProc;
	this->wc.cbClsExtra = 0;
	this->wc.cbWndExtra = 0;
	this->wc.hInstance = hInst;
	this->wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	this->wc.hCursor = LoadCursor(nullptr, IDC_IBEAM);
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
		int yStep = this->textMetrics_.tmHeight + this->textMetrics_.tmExternalLeading;

		ScrollWindow(this->hWnd, 0, -yStep * scrollIncrease, &this->textAreaEditRect_, &this->textAreaEditRect_);
		UpdateWindow(this->hWnd);

		this->vertScrollInfo_.nPos += scrollIncrease;
		SetScrollInfo(this->vertSBC_, SB_CTL, &this->vertScrollInfo_, TRUE);
		
		InvalidateRect(this->hWnd, &this->textAreaEditRect_, false);
		UpdateWindow(this->hWnd);
	}
}
void TextEditorWindow::UpdateHorzScroll(int scrollIncrease)
{

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
		//DrawText(deviceContext, line.c_str(), -1, &editorWindow->textAreaEditRect_, /*DT_CENTER | DT_VCENTER | DT_SINGLELINE*/DT_LEFT);
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
	scrollBoxWidth_(10)
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

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	this->SetTextMetrics();
	this->SetMyMenuSize(clientRect);
	this->SetTextAreaBorderRect(clientRect);
	this->SetTextAreaEditRect();

	this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("And what about  you?");
	this->textLines_.push_back("London is a capital of Great Britain");
	this->textLines_.push_back("No comments");
	this->textLines_.push_back("if(a > b && c < d) a = b;");

	this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("And what about  you?");
	this->textLines_.push_back("London is a capital of Great Britain");
	this->textLines_.push_back("No comments");
	this->textLines_.push_back("if(a > b && c < d) a = b;");
	this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("And what about  you?");
	this->textLines_.push_back("London is a capital of Great Britain");
	this->textLines_.push_back("No comments");
	this->textLines_.push_back("if(a > b && c < d) a = b;");
	this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("And what about  you?");
	this->textLines_.push_back("London is a capital of Great Britain");
	this->textLines_.push_back("No comments");
	this->textLines_.push_back("if(a > b && c < d) a = b;");
	this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("And what about  you?");
	this->textLines_.push_back("London is a capital of Great Britain");
	this->textLines_.push_back("No comments");
	this->textLines_.push_back("if(a > b && c < d) a = b;");
	this->textLines_.push_back("Hello everyone! My name is Ivan and I'm very glad to see you!");
	this->textLines_.push_back("And what about  you?");
	this->textLines_.push_back("London is a capital of Great Britain");
	this->textLines_.push_back("No comments");
	this->textLines_.push_back("if(a > b && c < d) a = b;");

	this->xStartWritePos_ = this->textAreaEditRect_.left;
	this->yStartWritePos_ = this->textAreaEditRect_.top;

	this->xCaret_ = 0;
	this->yCaret_ = 0;



	/* creating scroll bar controls*/
	
	//int sbcX = this->textAreaBorderRect_.left;
	//int sbcY = this->textAreaBorderRect_.top;
	//int sbcWidth = this->textAreaBorderRect_.right - textAreaBorderRect_.left;
	//int sbcHeight = this->textAreaBorderRect_.bottom - textAreaBorderRect_.top;

	// standard scroll box size;
	//int iHThumb = GetSystemMetrics(SM_CXHTHUMB);
	//int iVThumb = GetSystemMetrics(SM_CYVTHUMB);

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
	
	//SetScrollRange(hWndVertScroll, SB_CTL, 0, MAX_RANGE, FALSE);

	ShowWindow(this->hWnd, cmdShow);
}


HWND TextEditorWindow::GetHWnd()
{
	return hWnd;
}









map<HWND, TextEditorWindow*> TextEditorWindow::hwndTextEditorWndMap;

// window procedure definition;
LRESULT CALLBACK TextEditorWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TextEditorWindow* editorWindow = TextEditorWindow::hwndTextEditorWndMap[hWnd];

	switch (uMsg)
	{
	case WM_CREATE: {

	} break;


	case WM_SIZE: {
					  RECT newClientRect;
					  GetClientRect(hWnd, &newClientRect);

					  editorWindow->SetMyMenuSize(newClientRect);
					  editorWindow->SetTextAreaBorderRect(newClientRect);
					  editorWindow->SetTextAreaEditRect();
					  editorWindow->UpdateScrollBarControlsPosition();

					  editorWindow->UpdateScrollInfo();
	} break;

	case WM_PAINT: {
					   PAINTSTRUCT paintStruct;
					   HDC deviceContext = BeginPaint(hWnd, &paintStruct);

					   Rectangle(
						   deviceContext,
						   editorWindow->myMenuRect_.left,
						   editorWindow->myMenuRect_.top,
						   editorWindow->myMenuRect_.right,
						   editorWindow->myMenuRect_.bottom
						   );

					   Rectangle(
						   deviceContext,
						   editorWindow->textAreaBorderRect_.left,
						   editorWindow->textAreaBorderRect_.top,
						   editorWindow->textAreaBorderRect_.right,
						   editorWindow->textAreaBorderRect_.bottom
						   );



					   //RECT clientRect;
					   //GetClientRect(hWnd, &clientRect);
					   //
					   //int clientRectWidth = clientRect.right - clientRect.left;
					   //int clientRectHeight = clientRect.bottom - clientRect.top;
					   //
					   //string text = "";
					   //text = "Client Rectangle width = " + to_string(clientRectWidth)   + ", height = " + to_string(clientRectHeight);

					   
					   SelectObject(deviceContext, GetStockObject(SYSTEM_FIXED_FONT));

					   //for (int i = 0; i < editorWindow->textLines_.size(); ++i)
					   //{
						//   ExtTextOut(
						//	   deviceContext,
						//	   editorWindow->textAreaEditRect_.left,
						//	   editorWindow->textAreaEditRect_.top + i * editorWindow->textMetrics_.tmHeight,
						//	   ETO_CLIPPED,
						//	   &editorWindow->textAreaEditRect_,
						//	   editorWindow->textLines_[i].c_str(),
						//	   editorWindow->textLines_[i].length(), 
						//	   NULL
						//	   );
						//   //DrawText(deviceContext, line.c_str(), -1, &editorWindow->textAreaEditRect_, /*DT_CENTER | DT_VCENTER | DT_SINGLELINE*/DT_LEFT);
					   //}

					   editorWindow->PrintText(deviceContext);
					   
					   EndPaint(hWnd, &paintStruct);
	} break;

	case WM_LBUTTONDOWN: {
							 int cursorXPos = LOWORD(lParam);
							 int cursorYPos = HIWORD(lParam);

							 if (editorWindow->textAreaEditRect_.left < cursorXPos && cursorXPos < editorWindow->textAreaEditRect_.right &&
								 editorWindow->textAreaEditRect_.top < cursorYPos && cursorYPos < editorWindow->textAreaEditRect_.bottom)
							 {
								 const int offset = 2;
								 int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;
								 
								 int realLogicalX = (cursorXPos - editorWindow->textAreaEditRect_.left + offset) / editorWindow->textMetrics_.tmAveCharWidth;
								 int realLogicalY = (cursorYPos - editorWindow->textAreaEditRect_.top + offset) / yStep;


								 editorWindow->yCaret_ =
									 (realLogicalY < editorWindow->textLines_.size()) ? realLogicalY : editorWindow->textLines_.size() - 1;
								 
								 int currLineLen = editorWindow->textLines_[editorWindow->yCaret_].length();
								 
								 editorWindow->xCaret_ =
									 (realLogicalX < currLineLen) ? realLogicalX : currLineLen;
							 }

							 SendMessage(hWnd, WM_SETFOCUS, wParam, lParam);
	} break;

	case WM_SETFOCUS: {
						  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

						  CreateCaret(hWnd, NULL, 1, yStep);
						  SetCaretPos(
							  editorWindow->textMetrics_.tmAveCharWidth * editorWindow->xCaret_ + editorWindow->xStartWritePos_,
							  yStep * editorWindow->yCaret_ + editorWindow->yStartWritePos_
							  );
						  ShowCaret(hWnd);
	} break;

	case WM_KILLFOCUS: {
						   HideCaret(hWnd);
						   DestroyCaret();
	} break;

	case WM_VSCROLL: {
						 int scrollIncrease = 0;

						 switch (LOWORD(wParam)) 
						 {
						 case SB_LINEUP:
							 scrollIncrease = -1; 
							 break;

						 case SB_LINEDOWN:
							 scrollIncrease = 1; 
							 break;

						 case SB_PAGEUP:
							 scrollIncrease = -(int)editorWindow->vertScrollInfo_.nPage; 
							 break;

						 case SB_PAGEDOWN:
							 scrollIncrease = (int)editorWindow->vertScrollInfo_.nPage; 
							 break;

						 case SB_THUMBTRACK:
							 scrollIncrease = HIWORD(wParam) - editorWindow->vertScrollInfo_.nPos; 
							 break;

						 default: scrollIncrease = 0;
						 }

						 editorWindow->UpdateVertScroll(scrollIncrease);
	} break;

	case WM_KEYDOWN: {
						 int currXPos = editorWindow->xCaret_;
						 int currYPos = editorWindow->yCaret_;
						 
						 switch (wParam)
						 {
						 case VK_LEFT: {
										   if (currXPos == 0 && currYPos == 0)
											   break;

										   if (currXPos == 0)
										   {
											   editorWindow->xCaret_ = editorWindow->textLines_[currYPos - 1].length();
											   --editorWindow->yCaret_;
											   break;
										   }

										   --editorWindow->xCaret_;
						 } break;

						 case VK_RIGHT: {
											if (currXPos == editorWindow->textLines_[currYPos].length() &&
												currYPos == editorWindow->textLines_.size() - 1)
											{
												break;
											}

											if (currXPos == editorWindow->textLines_[currYPos].length())
											{
												editorWindow->xCaret_ = 0;
												++editorWindow->yCaret_;
												break;
											}

											++editorWindow->xCaret_;
						 } break;

						 case VK_UP: {

										 if (currYPos == 0)
										 {
											 break;
										 }

										 if (currXPos > editorWindow->textLines_[currYPos - 1].length())
										 {
											 editorWindow->xCaret_ = editorWindow->textLines_[currYPos - 1].length();
											 --editorWindow->yCaret_;
											 break;
										 }

										 --editorWindow->yCaret_;
						 } break;

						 case VK_DOWN: {
										   if (currYPos == editorWindow->textLines_.size() - 1)
										   {
											   break;
										   }

										   if (currXPos > editorWindow->textLines_[currYPos + 1].length())
										   {
											   editorWindow->xCaret_ = editorWindow->textLines_[currYPos + 1].length();
											   ++editorWindow->yCaret_;
											   break;
										   }
										   
										   ++editorWindow->yCaret_;
						 } break;

						 case VK_DELETE: {
											 if (currYPos == editorWindow->textLines_.size() - 1 &&
												 currXPos == editorWindow->textLines_[currYPos].length())
											 {
												 break;
											 }

											 if (currXPos == editorWindow->textLines_[currYPos].length())
											 {
												 editorWindow->textLines_[currYPos] += editorWindow->textLines_[currYPos + 1];

												 auto iter = editorWindow->textLines_.begin();
												 for (int i = 0; i < currYPos + 1; ++i)
												 {
													 ++iter;
												 }

												 editorWindow->textLines_.erase(iter);
											 }
											 else
											 {
												 editorWindow->textLines_[currYPos].erase(currXPos, 1);
											 }

											 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
						 } break;
						 }

						 int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

						 SetCaretPos(
							 editorWindow->xCaret_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
							 editorWindow->yCaret_ * yStep + editorWindow->yStartWritePos_
							 );
	} break;

	case WM_CHAR: {
					  int currXPos = editorWindow->xCaret_;
					  int currYPos = editorWindow->yCaret_;

					  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

					  switch (wParam)
					  {
					  case '\b': { // backspace;
									 if (currXPos == 0 && currYPos == 0)
										 break;

									 if (currXPos == 0)
									 {
										 --editorWindow->yCaret_;
										 editorWindow->xCaret_ = editorWindow->textLines_[currYPos - 1].length();

										 editorWindow->textLines_[currYPos - 1] += editorWindow->textLines_[currYPos];

										 auto iter = editorWindow->textLines_.begin();
										 for (int i = 0; i < currYPos; ++i)
										 {
											 ++iter;
										 }

										 editorWindow->textLines_.erase(iter);
									 }
									 else
									 {
										 editorWindow->textLines_[currYPos].erase(currXPos - 1, 1);
										 
										 --editorWindow->xCaret_;
									 }
									 
									 SetCaretPos(
										 editorWindow->xCaret_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
										 editorWindow->yCaret_ * yStep + editorWindow->yStartWritePos_
										 );

									 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
					  } break;

					  case '\t': {
									 do
									 {
									   SendMessage(hWnd, WM_CHAR, ' ', 1L);
									 } while (editorWindow->xCaret_ % 8 != 0);
					  } break;

					  case '\r': 
					  case '\n': { // new line character;
									 string textToMove = editorWindow->textLines_[currYPos].substr(currXPos);
									 editorWindow->textLines_[currYPos].erase(currXPos);

									 auto iter = editorWindow->textLines_.begin();
									 for (int i = 0; i < currYPos + 1; ++i)
									 {
										 ++iter;
									 }

									 editorWindow->textLines_.insert(iter, textToMove);

									 editorWindow->xCaret_ = 0;
									 ++editorWindow->yCaret_;

									 SetCaretPos(
										 editorWindow->xCaret_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
										 editorWindow->yCaret_ * yStep + editorWindow->yStartWritePos_
										 );

									 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
					  } break;

					  default: { // any other character;
								   char symbol = (char)wParam;
								   editorWindow->textLines_[currYPos].insert(currXPos, 1, symbol);

								   ++editorWindow->xCaret_;

								   SetCaretPos(
									   editorWindow->xCaret_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
									   editorWindow->yCaret_ * yStep + editorWindow->yStartWritePos_
									   );

								   InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
					  } break;
					  }
	} break;

	case WM_CLOSE: {
					   DestroyWindow(hWnd);
	} break;

	case WM_DESTROY:{
						PostQuitMessage(0);
	} break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}