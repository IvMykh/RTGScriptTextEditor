#pragma once
#include <Windows.h>

#include <map>
#include <string>
#include <vector>

#include "SelectedLogicalRegion.h"

// TODO: scrolling; ctrl+c; ctrl+v; ctrl+z;
// ! consider tm->tmExternalLeading later; 


class TextEditorWindow
{
	/*instance members*/
private: // fileds;
	WNDCLASSEX wc;
	HWND hWnd;
	
	// device context for selecting;
	HDC deviceContext_; // ! is got in WM_CREATE handler and is released in WM_DESTROY;


	HWND vertSBC_;
	SCROLLINFO vertScrollInfo_;

	HWND horzSBC_;
	SCROLLINFO horzScrollInfo_;

	const int margin_;
	const int textMargin_;
	const int myMenyHeight_;
	const int scrollBoxWidth_;

	RECT myMenuRect_;
	RECT textAreaBorderRect_;
	RECT textAreaEditRect_;

	std::vector<std::string> textLines_;

	int xStartWritePos_;
	int yStartWritePos_;

	/* 
	logical caret position on the text edit area; can be both positive and negative;
	*/
	int xCaretEditAreaPos_;
	int yCaretEditAreaPos_;
	
	bool isCaretVisible_;

	/*
	logical caret position in terms of text lines and characters positions;
	*/
	int xCaretTextPosition_;
	int yCaretTextPosition_;

	TEXTMETRIC textMetrics_;

	// selecting text;
	SelectedTextRegion selTextReg_;
	bool isMouseTracked_;


private: // methods;
	void RegisterWindowClass(char* className, UINT classStyle, LRESULT(WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM), HINSTANCE hInst);
	
	void SetMyMenuSize(const RECT& clientRect);
	void SetTextAreaBorderRect(const RECT& clientRect);
	void SetTextAreaEditRect();
	void SetTextMetrics();
	
	void UpdateScrollBarControlsPosition();
	void UpdateScrollInfo();

	void UpdateVertScroll(int scrollIncrease);
	void UpdateHorzScroll(int scrollIncrease);

	void UpdateCaretPosition();

	void PrintText(HDC deviceContext);
	//
	void ShowSelectedText(const bool shouldBeColored);
	//


	//void AdjustWhenCaretInvisible();


public: // methods;
	TextEditorWindow(
		LPCSTR windowName,
		HINSTANCE hInst,
		int cmdShow,
		LRESULT(WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM),
		LPCSTR menuName = nullptr,
		int x = CW_USEDEFAULT,
		int y = 0,
		int width = CW_USEDEFAULT,
		int height = 0,
		UINT classStyle = CS_HREDRAW | CS_VREDRAW,
		DWORD windowStyle = WS_OVERLAPPEDWINDOW /*| WS_HSCROLL | WS_VSCROLL*/,
		HWND hParent = nullptr 
	);

	HWND GetHWnd();


	~TextEditorWindow();

	/*static members*/
private: // fields;
	static std::map<HWND, TextEditorWindow*> hwndTextEditorWndMap;

public: // methods;
	// window procedure;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};

