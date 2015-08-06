#include "TextEditorWindow.h"

using namespace std;

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

					   SelectObject(deviceContext, GetStockObject(SYSTEM_FIXED_FONT));
					   editorWindow->PrintText(deviceContext);

					   


					   //
					   //ShowCaret(hWnd);
					   //
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

								 //
								 int diff = editorWindow->textLines_.size() - editorWindow->vertScrollInfo_.nPos;
								 int nLinesDisplayed = min(diff, editorWindow->vertScrollInfo_.nPage);
								 //

								 editorWindow->yCaretEditAreaPos_ =
									 (realLogicalY < nLinesDisplayed) ? realLogicalY : nLinesDisplayed - 1;

								 // y-caret text position;
								 editorWindow->yCaretTextPosition_ = editorWindow->vertScrollInfo_.nPos + editorWindow->yCaretEditAreaPos_;


								 int currLineLen = editorWindow->textLines_[editorWindow->yCaretTextPosition_].length();

								 editorWindow->xCaretEditAreaPos_ =
									 (realLogicalX < currLineLen) ? realLogicalX : currLineLen;

								 // x-caret text position;
								 editorWindow->xCaretTextPosition_ = editorWindow->xCaretEditAreaPos_;
							 }

							 SendMessage(hWnd, WM_SETFOCUS, wParam, lParam);
	} break;

	case WM_SETFOCUS: {
						  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

						  CreateCaret(hWnd, NULL, 1, yStep);
						  SetCaretPos(
							  editorWindow->textMetrics_.tmAveCharWidth * editorWindow->xCaretEditAreaPos_ + editorWindow->xStartWritePos_,
							  yStep * editorWindow->yCaretEditAreaPos_ + editorWindow->yStartWritePos_
							  );

						  if (editorWindow->yCaretEditAreaPos_ >= 0 && 
							  editorWindow->yCaretEditAreaPos_ < editorWindow->vertScrollInfo_.nPage)
						  {
							  editorWindow->isCaretVisible_ = true;
						  }

						  if (editorWindow->isCaretVisible_)
						  {
							  ShowCaret(hWnd);
						  }
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


						 //if (scrollIncrease != 0)
						 //{
						//	 editorWindow->yCaretEditAreaPos_ -= scrollIncrease;
						 //
						//	 if (editorWindow->isCaretVisible_ && 
						//		 (editorWindow->yCaretEditAreaPos_ < 0 || editorWindow->yCaretEditAreaPos_ >= editorWindow->vertScrollInfo_.nPage))
						//	 {
						//		 editorWindow->isCaretVisible_ = false;
						//		 HideCaret(hWnd);
						//	 }
						//	 else if (!editorWindow->isCaretVisible_ &&
						//		 editorWindow->yCaretEditAreaPos_ >= 0 && 
						//		 editorWindow->yCaretEditAreaPos_ < editorWindow->vertScrollInfo_.nPage)
						//	 {
						//		 editorWindow->isCaretVisible_ = true;
						//		 ShowCaret(hWnd);
						//	 }
						 //}
						 //
						 //editorWindow->UpdateVertScroll(scrollIncrease);
						 //editorWindow->UpdateCaretPosition();
						 //
						 //if (editorWindow->isCaretVisible_)
						 //{
						//	 ShowCaret(hWnd);
						 //}

						 if (scrollIncrease != 0)
						 {
							 //editorWindow->UpdateCaretPosition();
							 editorWindow->UpdateVertScroll(scrollIncrease);
						 
						 	 if (editorWindow->yCaretEditAreaPos_ < 0 || editorWindow->yCaretEditAreaPos_ >= editorWindow->vertScrollInfo_.nPage)
						 	 {
						 		 editorWindow->isCaretVisible_ = false;
						 		 HideCaret(hWnd);
						 	 }
						 	 else
						 	 {
						 		 editorWindow->isCaretVisible_ = true;
						 		 ShowCaret(hWnd);
						 	 }
						 }
	} break;

	case WM_KEYDOWN: {
						 int currXEditAreaPos = editorWindow->xCaretEditAreaPos_;
						 int currYEditAreaPos = editorWindow->yCaretEditAreaPos_;

						 int currYTextPos = editorWindow->yCaretTextPosition_;
						 int currXTextPos = editorWindow->xCaretTextPosition_;

						 switch (wParam)
						 {
						 case VK_LEFT: {
										   if (currXTextPos == 0 && currYTextPos == 0)
											   break;

										   if (currXTextPos == 0)
										   {
											   editorWindow->xCaretTextPosition_ = editorWindow->textLines_[currYTextPos - 1].length();
											   --editorWindow->yCaretTextPosition_;

											   editorWindow->xCaretEditAreaPos_ = editorWindow->xCaretTextPosition_;
											   --editorWindow->yCaretEditAreaPos_;
											   break;
										   }

										   --editorWindow->xCaretTextPosition_;
										   --editorWindow->xCaretEditAreaPos_;
						 } break;

						 case VK_RIGHT: {
											if (currXTextPos == editorWindow->textLines_[currYTextPos].length() &&
												currYTextPos == editorWindow->textLines_.size() - 1)
											{
												break;
											}

											if (currXTextPos == editorWindow->textLines_[currYTextPos].length())
											{
												editorWindow->xCaretEditAreaPos_ = 0;
												editorWindow->xCaretTextPosition_ = 0;

												++editorWindow->yCaretEditAreaPos_;
												++editorWindow->yCaretTextPosition_;
												break;
											}

											++editorWindow->xCaretEditAreaPos_;
											++editorWindow->xCaretTextPosition_;
						 } break;

						 case VK_UP: {
										 if (currYTextPos == 0)
										 {
											 break;
										 }

										 if (currXTextPos > editorWindow->textLines_[currYTextPos - 1].length())
										 {
											 editorWindow->xCaretTextPosition_ = editorWindow->textLines_[currYTextPos - 1].length();
											 --editorWindow->yCaretTextPosition_;

											 --editorWindow->yCaretEditAreaPos_;
											 editorWindow->xCaretEditAreaPos_ = editorWindow->xCaretTextPosition_;

											 break;
										 }

										 --editorWindow->yCaretTextPosition_;
										 --editorWindow->yCaretEditAreaPos_;
						 } break;

						 case VK_DOWN: {
										   if (currYTextPos == editorWindow->textLines_.size() - 1)
										   {
											   break;
										   }

										   if (currXTextPos > editorWindow->textLines_[currYTextPos + 1].length())
										   {
											   editorWindow->xCaretTextPosition_ = editorWindow->textLines_[currYTextPos + 1].length();
											   ++editorWindow->yCaretTextPosition_;

											   ++editorWindow->yCaretEditAreaPos_;
											   editorWindow->xCaretEditAreaPos_ = editorWindow->xCaretTextPosition_;

											   break;
										   }

										   ++editorWindow->yCaretTextPosition_;
										   ++editorWindow->yCaretEditAreaPos_;
						 } break;

						 case VK_DELETE: {
											 if (currYTextPos == editorWindow->textLines_.size() - 1 &&
												 currXTextPos == editorWindow->textLines_[currYTextPos].length())
											 {
												 break;
											 }

											 if (currXTextPos == editorWindow->textLines_[currYTextPos].length())
											 {
												 editorWindow->textLines_[currYTextPos] += editorWindow->textLines_[currYTextPos + 1];

												 auto iter = editorWindow->textLines_.begin();
												 for (int i = 0; i < currYTextPos + 1; ++i)
												 {
													 ++iter;
												 }

												 editorWindow->textLines_.erase(iter);
											 }
											 else
											 {
												 editorWindow->textLines_[currYTextPos].erase(currXTextPos, 1);
											 }

											 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
						 } break;
						 }

						 int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

						 SetCaretPos(
							 editorWindow->xCaretEditAreaPos_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
							 editorWindow->yCaretEditAreaPos_ * yStep + editorWindow->yStartWritePos_
							 );

						 ShowCaret(hWnd);
	} break;

	case WM_CHAR: {
					  int currXEditAreaPos = editorWindow->xCaretEditAreaPos_;
					  int currYEditAreaPos = editorWindow->yCaretEditAreaPos_;

					  int currYTextPos = editorWindow->yCaretTextPosition_;
					  int currXTextPos = editorWindow->xCaretTextPosition_;

					  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

					  switch (wParam)
					  {
					  case '\b': { // backspace;
									 if (currXTextPos == 0 && currYTextPos == 0)
										 break;

									 if (currXTextPos == 0)
									 {
										 --editorWindow->yCaretTextPosition_;
										 --editorWindow->yCaretEditAreaPos_;

										 editorWindow->xCaretEditAreaPos_ = editorWindow->textLines_[currYTextPos - 1].length();
										 editorWindow->xCaretTextPosition_ = editorWindow->textLines_[currYTextPos - 1].length();

										 editorWindow->textLines_[currYTextPos - 1] += editorWindow->textLines_[currYTextPos];

										 auto iter = editorWindow->textLines_.begin();
										 for (int i = 0; i < currYTextPos; ++i)
										 {
											 ++iter;
										 }

										 editorWindow->textLines_.erase(iter);

										 editorWindow->UpdateScrollInfo();
									 }
									 else
									 {
										 editorWindow->textLines_[currYTextPos].erase(currXTextPos - 1, 1);

										 --editorWindow->xCaretTextPosition_;
										 --editorWindow->xCaretEditAreaPos_;
									 }

									 SetCaretPos(
										 editorWindow->xCaretEditAreaPos_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
										 editorWindow->yCaretEditAreaPos_ * yStep + editorWindow->yStartWritePos_
										 );

									 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
					  } break;

					  case '\t': {
									 do
									 {
										 SendMessage(hWnd, WM_CHAR, ' ', 1L);
									 } while (editorWindow->xCaretEditAreaPos_ % 8 != 0);
					  } break;

					  case '\r':
					  case '\n': { // new line character;
									 string textToMove = editorWindow->textLines_[currYTextPos].substr(currXTextPos);
									 editorWindow->textLines_[currYTextPos].erase(currXTextPos);

									 auto iter = editorWindow->textLines_.begin();
									 for (int i = 0; i < currYTextPos + 1; ++i)
									 {
										 ++iter;
									 }

									 editorWindow->textLines_.insert(iter, textToMove);

									 editorWindow->UpdateScrollInfo();

									 editorWindow->xCaretEditAreaPos_ = 0;
									 editorWindow->xCaretTextPosition_ = 0;

									 ++editorWindow->yCaretEditAreaPos_;
									 ++editorWindow->yCaretTextPosition_;

									 SetCaretPos(
										 editorWindow->xCaretEditAreaPos_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
										 editorWindow->yCaretEditAreaPos_ * yStep + editorWindow->yStartWritePos_
										 );

									 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
					  } break;

					  default: { // any other character;
								   char symbol = (char)wParam;
								   editorWindow->textLines_[currYTextPos].insert(currXTextPos, 1, symbol);

								   ++editorWindow->xCaretEditAreaPos_;
								   ++editorWindow->xCaretTextPosition_;

								   SetCaretPos(
									   editorWindow->xCaretEditAreaPos_ * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_,
									   editorWindow->yCaretEditAreaPos_ * yStep + editorWindow->yStartWritePos_
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