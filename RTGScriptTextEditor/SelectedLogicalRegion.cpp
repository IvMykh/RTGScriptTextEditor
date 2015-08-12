#include "SelectedLogicalRegion.h"

#include <utility>

using namespace std;


SelectedTextRegion::SelectedTextRegion():
	logicUp_(0),
	logicLeft_(0),
	logicDown(0),
	logicRight_(0)
{
}

SelectedTextRegion::SelectedTextRegion(const int lu, const int ll, const int ld, const int lr) :
	logicUp_(lu),
	logicLeft_(ll),
	logicDown(ld),
	logicRight_(lr)
{
}



bool SelectedTextRegion::IsRegionEmpty() const
{
	return this->logicUp_ == this->logicDown
		&& this->logicLeft_ == this->logicRight_;
}

void SelectedTextRegion::SetRegionEmpty(const int caretX, const int caretY)
{
	this->logicUp_		=	caretY;
	this->logicDown		=	caretY;

	this->logicLeft_	=	caretX;
	this->logicRight_	=	caretX;
}

const SelectedTextRegion SelectedTextRegion::AdjustBounds() const
{
	SelectedTextRegion finalRegion = *this;

	if (finalRegion.logicDown < finalRegion.logicUp_)
	{
		swap(finalRegion.logicDown, finalRegion.logicUp_);
		swap(finalRegion.logicRight_, finalRegion.logicLeft_);
	}

	if (finalRegion.logicUp_ == finalRegion.logicDown && finalRegion.logicLeft_ > finalRegion.logicRight_)
	{
		swap(finalRegion.logicLeft_, finalRegion.logicRight_);
	}

	return finalRegion;
}