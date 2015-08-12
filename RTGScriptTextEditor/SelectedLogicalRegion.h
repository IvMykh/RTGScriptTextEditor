#pragma once
class SelectedTextRegion
{
public: // fields;
	int logicUp_;
	int logicLeft_;
	int logicDown;
	int logicRight_;

public: // methods;
	SelectedTextRegion();
	SelectedTextRegion(const int lu, const int ll, const int ld, const int lr);

	bool IsRegionEmpty() const;

	void SetRegionEmpty(const int caretX, const int caretY);

	/* adjusts bounds when text is selected from down/right to up/left */
	const SelectedTextRegion AdjustBounds() const;
};