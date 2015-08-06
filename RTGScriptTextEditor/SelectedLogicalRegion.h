#pragma once
class SelectedTextRegion
{
public: // fields;
	int logicUp_;
	int logicLeft_;
	int logicDown;
	int logicRight_;

public: // methods;
	SelectedTextRegion(const int lu, const int ll, const int ld, const int lr);
};