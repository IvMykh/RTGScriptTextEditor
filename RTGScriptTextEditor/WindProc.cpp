#include "TextEditorWindow.h"

#include <algorithm>
#include <zmouse.h>

using namespace std;



// window procedure definition;
LRESULT CALLBACK TextEditorWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TextEditorWindow* editorWindow = TextEditorWindow::hwndTextEditorWndMap[hWnd];


	switch (uMsg)
	{
	case WM_CREATE: {} break;

	case WM_SIZE: {
					  RECT newClientRect;
					  GetClientRect(hWnd, &newClientRect);

					  editorWindow->SetMyMenuSize(newClientRect);
					  editorWindow->SetTextAreaBorderRect(newClientRect);
					  editorWindow->SetTextAreaEditRect();
					  editorWindow->UpdateScrollBarControlsPosition();

					  editorWindow->UpdateScrollInfo();


					  int currXEditAreaPos = editorWindow->xCaretEditAreaPos_;
					  int currYEditAreaPos = editorWindow->yCaretEditAreaPos_;
					  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

					  int xCursorPos = currXEditAreaPos * editorWindow->textMetrics_.tmAveCharWidth + editorWindow->xStartWritePos_;
					  int yCursorPos = currYEditAreaPos * yStep + editorWindow->yStartWritePos_;

					  if (!editorWindow->IsCaretInTextAreaEditRect(xCursorPos, yCursorPos) 
						  && editorWindow->isCaretVisible_)
					  {
						  editorWindow->isCaretVisible_ = false;
						  HideCaret(hWnd);
						  break;
					  }

					  if (editorWindow->IsCaretInTextAreaEditRect(xCursorPos, yCursorPos) && !editorWindow->isCaretVisible_)
					  {
						  editorWindow->isCaretVisible_ = true;
						  ShowCaret(hWnd);
						  break;
					  }
	} break;
	
	case WM_GETMINMAXINFO: {
							   MINMAXINFO* mmi = (MINMAXINFO*)lParam;
							   mmi->ptMinTrackSize.x = 200;
							   mmi->ptMinTrackSize.y = 200;
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

					   EndPaint(hWnd,  &paintStruct);
					   
					   editorWindow->ShowSelectedText(true);
	} break;

	case WM_LBUTTONDOWN: {

							 int cursorXPos = LOWORD(lParam);
							 int cursorYPos = HIWORD(lParam);

							 if (editorWindow->IsCaretInTextAreaBorderRect(cursorXPos, cursorYPos))
							 {
								 /*
								 disselect text;
								 */
								 
								 /*hide to remove invalid caret tracks*/
								 HideCaret(hWnd);

								 editorWindow->ShowSelectedText(false);


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



								 // text selection;
								 if (!editorWindow->isMouseTracked_)
								 { 
									 /*
									 GetDC() here and manually fill PAINTSTRUCT;
									 */

									 if (editorWindow->deviceContext_ == nullptr)
									 {
										 editorWindow->deviceContext_ = GetDC(hWnd);
									 }

									 editorWindow->selTextReg_
										 .SetRegionEmpty(editorWindow->xCaretTextPosition_, editorWindow->yCaretTextPosition_);

									 //editorWindow->selTextReg_.logicUp_		=	editorWindow->yCaretTextPosition_;
									 //editorWindow->selTextReg_.logicLeft_	=	editorWindow->xCaretTextPosition_;
									 //editorWindow->selTextReg_.logicDown	= editorWindow->selTextReg_.logicUp_;
									 //editorWindow->selTextReg_.logicRight_	= editorWindow->selTextReg_.logicLeft_;


									 editorWindow->isMouseTracked_ = true;
								 }
								 SendMessage(hWnd, WM_SETFOCUS, wParam, lParam);
							 }

	} break;

	case WM_MOUSEMOVE: {
						   int cursorXPos = LOWORD(lParam);
						   int cursorYPos = HIWORD(lParam);

						   if (editorWindow->IsCaretInTextAreaBorderRect(cursorXPos, cursorYPos))
						   {
							   SetCursor(LoadCursor(NULL, IDC_IBEAM));
						   }
						   else
						   {
							   SetCursor(LoadCursor(NULL, IDC_ARROW));
						   }

						   if (editorWindow->isMouseTracked_)
						   {
							   HideCaret(hWnd); // think about it;
							   SendMessage(hWnd, WM_LBUTTONDOWN, wParam, lParam);


							   editorWindow->selTextReg_.logicDown = editorWindow->yCaretTextPosition_;
							   editorWindow->selTextReg_.logicRight_ = editorWindow->xCaretTextPosition_;

							   editorWindow->ShowSelectedText(true);
							   
							   ShowCaret(hWnd);
						   }
	} break;

	case WM_LBUTTONUP: {
						   editorWindow->isMouseTracked_ = false;
	} break;

	case WM_SETFOCUS: {
						  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;

						  CreateCaret(hWnd, NULL, 1, yStep);

						  editorWindow->UpdateCaretPosition();

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

						 if (scrollIncrease != 0)
						 {
							 editorWindow->UpdateVertScroll(scrollIncrease);
							 editorWindow->UpdateCaretPosition();
						 
						 	 if ((editorWindow->yCaretEditAreaPos_ < 0 || editorWindow->yCaretEditAreaPos_ >= editorWindow->vertScrollInfo_.nPage) &&
								 editorWindow->isCaretVisible_)
						 	 {
						 		 editorWindow->isCaretVisible_ = false;
						 		 HideCaret(hWnd);
								 break;
						 	 }
						 	 
							 if (editorWindow->yCaretEditAreaPos_ >= 0 && 
								 editorWindow->yCaretEditAreaPos_ < editorWindow->vertScrollInfo_.nPage &&
								 !editorWindow->isCaretVisible_)
						 	 {
						 		 editorWindow->isCaretVisible_ = true;
						 		 ShowCaret(hWnd);
						 	 }

						 }
	} break;

	case WM_MOUSEWHEEL: {
								int scrollIncrease = -3 * (short)HIWORD(wParam) / WHEEL_DELTA;

								if (scrollIncrease > 0)
								{
									int maxScrollPos = editorWindow->textLines_.size() - editorWindow->vertScrollInfo_.nPage;

									while (scrollIncrease != 0 && (editorWindow->vertScrollInfo_.nPos + scrollIncrease) > maxScrollPos)
									{
										--scrollIncrease;
									}
								}
								else
								{
									while (scrollIncrease != 0 && (editorWindow->vertScrollInfo_.nPos + scrollIncrease) < 0)
									{
										++scrollIncrease;
									}
								}

								if (scrollIncrease != 0)
								{
									int myWParam = MAKELONG(SB_THUMBTRACK, editorWindow->vertScrollInfo_.nPos + scrollIncrease);
									SendMessage(hWnd, WM_VSCROLL, myWParam, NULL);
								}
	} break;
	
	case WM_COMMAND: {
						 switch (LOWORD(wParam))
						 {
						 case  ID_CTRL_A: {
											  /* move caret to the end of text and adjust scrolling */
											  
											  editorWindow->xCaretEditAreaPos_ = editorWindow->textLines_[editorWindow->textLines_.size() - 1].length();
											  editorWindow->yCaretEditAreaPos_ = editorWindow->textLines_.size() - 1 - editorWindow->vertScrollInfo_.nPos;
																									/* ! pay attention to "- editorWindow->vertScrollInfo_.nPos" */


											  editorWindow->xCaretTextPosition_ = editorWindow->textLines_[editorWindow->textLines_.size() - 1].length();
											  editorWindow->yCaretTextPosition_ = editorWindow->textLines_.size() - 1;

											  editorWindow->AdjustVertScrollingToCaretLine();
											  editorWindow->UpdateCaretPosition();

											  /* at this point caret is surely visible */
											  editorWindow->isCaretVisible_ = true;
											  ShowCaret(hWnd);

											  editorWindow->selTextReg_.logicUp_ = 0;
											  editorWindow->selTextReg_.logicLeft_ = 0;
											  editorWindow->selTextReg_.logicDown = editorWindow->textLines_.size() - 1;
											  editorWindow->selTextReg_.logicRight_ = editorWindow->textLines_[editorWindow->textLines_.size() - 1].length();
											  
											  if (editorWindow->deviceContext_ == nullptr)
											  {
												  editorWindow->deviceContext_ = GetDC(hWnd);
											  }

											  editorWindow->ShowSelectedText(true);
						 } break;

						 case  ID_CTRL_C: {
											  if (editorWindow->selTextReg_.IsRegionEmpty())
												  break;

											  SelectedTextRegion adjustedRegion = editorWindow->selTextReg_.AdjustBounds();
											  editorWindow->CopySelectedTextToClipboard(adjustedRegion);
						 } break;

						 case  ID_CTRL_V: {
											  /* opening clipboard for getting text */
											  if (!OpenClipboard(hWnd))
											  {
												  MessageBox(nullptr, "Cannot open clipboard for pasting", "Error", MB_OK);
												  break;
											  }


											  /*figuring out if there is available text for getting*/
											  HANDLE hClipMemory = GetClipboardData(CF_TEXT);

											  if (hClipMemory == nullptr)
											  {
												  if (!CloseClipboard())
													  MessageBox(nullptr, "Cannot close clipboard", "Error", MB_OK);

												  break;
											  }


											  /*saving data to string*/
											  char* cstrData = (char*)GlobalLock(hClipMemory);
											  string myText(cstrData);
											  
											  GlobalUnlock(hClipMemory);

											  if (!CloseClipboard())
											  {
												  MessageBox(nullptr, "Cannot close clipboard while pasting", "Error", MB_OK);
												  break;
											  }


											  /*dealing with string of text itself*/
											  
											  // splitting & dealing with tabs;
											  vector<string> textLines;
											  const char delim = '\n';
											  
											  bool endsWithDelim = SplitString(myText, delim, textLines);
											  ConvertTabsIntoSpaces(textLines);


											  if (textLines.size() == 1)
											  {
												  if (endsWithDelim)
												  {
													  string lastLineEnding = editorWindow->textLines_[editorWindow->yCaretTextPosition_]
														  .substr(editorWindow->xCaretTextPosition_, string::npos);

													  editorWindow->textLines_[editorWindow->yCaretTextPosition_]
														  .erase(editorWindow->xCaretTextPosition_, string::npos);



													  editorWindow->textLines_[editorWindow->yCaretTextPosition_] += textLines.front();

													  editorWindow->textLines_
														  .insert(
														  editorWindow->textLines_.begin() + editorWindow->yCaretTextPosition_ + 1,
														  lastLineEnding
														  );

													  /*caret*/
													  ++editorWindow->yCaretTextPosition_;
													  ++editorWindow->yCaretEditAreaPos_;

													  editorWindow->xCaretTextPosition_ = 0;//lastLineEnding.length();
													  editorWindow->xCaretEditAreaPos_	= 0;//lastLineEnding.length();
													  /**/
												  }
												  else
												  {

													  editorWindow->textLines_[editorWindow->yCaretTextPosition_]
														  .insert(editorWindow->xCaretTextPosition_, textLines.front());

													  /*caret*/
													  editorWindow->xCaretTextPosition_	+= textLines.front().length();
													  editorWindow->xCaretEditAreaPos_	+= textLines.front().length();
													  /**/
												  }
											  }
											  else		// if (textLines.size() > 1)
											  {
												  string lastLineEnding = editorWindow->textLines_[editorWindow->yCaretTextPosition_]
													  .substr(editorWindow->xCaretTextPosition_, string::npos);

												  editorWindow->textLines_[editorWindow->yCaretTextPosition_]
													  .erase(editorWindow->xCaretTextPosition_, string::npos);



												  editorWindow->textLines_[editorWindow->yCaretTextPosition_] += textLines.front();


												  auto begIter = textLines.begin() + 1;
												  auto endIter = textLines.end();

												  int offset = editorWindow->yCaretTextPosition_;

												  while (begIter != endIter)
												  {
													  editorWindow->textLines_.insert(editorWindow->textLines_.begin() + ++offset, *begIter++);
												  }


												  if (endsWithDelim)
												  {
													  editorWindow->textLines_.insert(editorWindow->textLines_.begin() + ++offset, lastLineEnding);

													  /*caret*/
													  int linesInserted = textLines.size();

													  editorWindow->yCaretTextPosition_ += linesInserted;
													  editorWindow->yCaretEditAreaPos_ += linesInserted;

													  editorWindow->xCaretTextPosition_ = 0; //lastLineEnding.length();
													  editorWindow->xCaretEditAreaPos_ = 0;	//lastLineEnding.length();
													  /**/
												  }
												  else
												  {
													  /*caret*/
													  int linesInserted = textLines.size() - 1;

													  editorWindow->yCaretTextPosition_ += linesInserted;
													  editorWindow->yCaretEditAreaPos_ += linesInserted;

													  editorWindow->xCaretTextPosition_ = editorWindow->textLines_[offset].length();
													  editorWindow->xCaretEditAreaPos_ = editorWindow->textLines_[offset].length();
													  /**/

													  editorWindow->textLines_[offset] += lastLineEnding;
												  }
											  }

											  editorWindow->UpdateScrollInfo();
											  editorWindow->AdjustVertScrollingToCaretLine();
											  editorWindow->UpdateCaretPosition();

											  InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
						 } break;

						 case  ID_CTRL_X: {
											  if (editorWindow->selTextReg_.IsRegionEmpty())
												  break;

											  SelectedTextRegion adjustedRegion = editorWindow->selTextReg_.AdjustBounds();
											  
											  if (!editorWindow->CopySelectedTextToClipboard(adjustedRegion))
											  {
												  MessageBox(nullptr, "Cannot end cutting successfully", "!", MB_OK);
												  break;
											  }

											  editorWindow->RemoveSelectedTextRegion(adjustedRegion);

						 } break;

						 default:
							 break;
						 }
	} break;

	case WM_KEYDOWN: {
						 if (wParam == VK_CONTROL || wParam == VK_SHIFT)
							 break;

						 
						 if (editorWindow->deviceContext_ == nullptr)
						 {
							 editorWindow->deviceContext_ = GetDC(hWnd);
						 }


						 /*
						 Adjusting when caret out of edit rect;
						 (BE CAREFUL WHILE HORZ SCROLLING ADJUSTING)
						 */

						 editorWindow->AdjustVertScrollingToCaretLine();

						 /*
						 */


						 /*be careful while modifying actual values*/

						 int currYTextPos = editorWindow->yCaretTextPosition_;
						 int currXTextPos = editorWindow->xCaretTextPosition_;

						 int currXEditAreaPos = editorWindow->xCaretEditAreaPos_;
						 int currYEditAreaPos = editorWindow->yCaretEditAreaPos_;



						 switch (wParam)
						 {
						 case VK_LEFT: {
										   if (currXTextPos == 0 && currYTextPos == 0)
											   break;

										   
										   if (currXTextPos == 0 && currYTextPos != 0 && currYEditAreaPos == 0)
										   {
											   SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
										   }

										   /*new*/
										   editorWindow->ShowSelectedText(false);

										   editorWindow->selTextReg_.SetRegionEmpty(
											   editorWindow->xCaretTextPosition_,
											   editorWindow->yCaretTextPosition_
											   );
										   /**/

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

											if (currXTextPos == editorWindow->textLines_[currYTextPos].length() && 
												currYEditAreaPos == editorWindow->vertScrollInfo_.nPage - 1)
											{
												SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
											}

											/*new*/
											editorWindow->ShowSelectedText(false);

											editorWindow->selTextReg_.SetRegionEmpty(
												editorWindow->xCaretTextPosition_,
												editorWindow->yCaretTextPosition_
												);
											/**/

											if (currXTextPos == editorWindow->textLines_[currYTextPos].length())
											{
												editorWindow->xCaretEditAreaPos_ = 0;
												editorWindow->xCaretTextPosition_ = 0;

												++editorWindow->yCaretEditAreaPos_;
												++editorWindow->yCaretTextPosition_;

												/*new*/
												/*editorWindow->ShowSelectedText(false);

												editorWindow->selTextReg_.SetRegionEmpty(
													editorWindow->xCaretTextPosition_,
													editorWindow->yCaretTextPosition_
													);*/
												/**/

												break;
											}

											++editorWindow->xCaretEditAreaPos_;
											++editorWindow->xCaretTextPosition_;

											/*new*/
											/*editorWindow->ShowSelectedText(false);
											
											editorWindow->selTextReg_.SetRegionEmpty(
												editorWindow->xCaretTextPosition_,
												editorWindow->yCaretTextPosition_
												);*/
											/**/
						 } break;

						 case VK_UP: {
										 if (currYTextPos == 0)
										 {
											 break;
										 }


										 if (currYEditAreaPos == 0)
										 {
											 SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
										 }

										 /*new*/
										 editorWindow->ShowSelectedText(false);

										 editorWindow->selTextReg_.SetRegionEmpty(
											 editorWindow->xCaretTextPosition_,
											 editorWindow->yCaretTextPosition_
											 );
										 /**/


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


										   if (currYEditAreaPos == editorWindow->vertScrollInfo_.nPage - 1)
										   {
											   SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
										   }


										   /*new*/
										   editorWindow->ShowSelectedText(false);

										   editorWindow->selTextReg_.SetRegionEmpty(
											   editorWindow->xCaretTextPosition_,
											   editorWindow->yCaretTextPosition_
											   );
										   /**/


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
											 if (editorWindow->selTextReg_.IsRegionEmpty())
											 {
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

													 editorWindow->UpdateScrollInfo();
												 }
												 else
												 {
													 editorWindow->textLines_[currYTextPos].erase(currXTextPos, 1);
												 }

												 InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
											 }
											 else
											 {
												 SelectedTextRegion adjustedRegion = editorWindow->selTextReg_.AdjustBounds();
												 editorWindow->RemoveSelectedTextRegion(adjustedRegion);
											 }
						 } break;

						 case VK_ESCAPE: {
											 PostMessage(hWnd, WM_DESTROY, NULL, NULL);
						 } break;
						 }

						 editorWindow->UpdateCaretPosition();
						 ShowCaret(hWnd);
	} break;
	
	case WM_CHAR: {  
					  bool isSelRegRemoved = false;

					  if (!editorWindow->selTextReg_.IsRegionEmpty())
					  {
						  SelectedTextRegion adjustedRegion = editorWindow->selTextReg_.AdjustBounds();
						  editorWindow->RemoveSelectedTextRegion(adjustedRegion);

						  isSelRegRemoved = true;
					  }


					  int currXEditAreaPos = editorWindow->xCaretEditAreaPos_;
					  int currYEditAreaPos = editorWindow->yCaretEditAreaPos_;

					  int currYTextPos = editorWindow->yCaretTextPosition_;
					  int currXTextPos = editorWindow->xCaretTextPosition_;

					  int yStep = editorWindow->textMetrics_.tmHeight + editorWindow->textMetrics_.tmExternalLeading;



					  switch (wParam)
					  {
					  case '\b': { // backspace;

									 if (!isSelRegRemoved)
									 {	/* delete single char */
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

											 SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
											 editorWindow->textLines_.erase(iter);


											 editorWindow->UpdateScrollInfo();
										 }
										 else
										 {	
											 editorWindow->textLines_[currYTextPos].erase(currXTextPos - 1, 1);

											 --editorWindow->xCaretTextPosition_;
											 --editorWindow->xCaretEditAreaPos_;
										 }
									 }
									 // oposite case already handled;
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

									 /* new */
									 if (currYEditAreaPos == editorWindow->vertScrollInfo_.nPage - 1)
									 {
										 SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
									 }
									 /**/

									 editorWindow->xCaretEditAreaPos_ = 0;
									 editorWindow->xCaretTextPosition_ = 0;

									 ++editorWindow->yCaretEditAreaPos_;
									 ++editorWindow->yCaretTextPosition_;
					  } break;

					  default: { // any other character;
								   char symbol = (char)wParam;

								   editorWindow->textLines_[currYTextPos].insert(currXTextPos, 1, symbol);

								   ++editorWindow->xCaretEditAreaPos_;
								   ++editorWindow->xCaretTextPosition_;
					  } break;
					  }

					  editorWindow->UpdateCaretPosition();
					  ShowCaret(hWnd);

					  InvalidateRect(hWnd, &editorWindow->textAreaEditRect_, TRUE);
	} break;

	case WM_CLOSE: {
					   DestroyWindow(hWnd);
	} break;

	case WM_DESTROY:{
						ReleaseDC(hWnd, editorWindow->deviceContext_);
						PostQuitMessage(0);
	} break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}