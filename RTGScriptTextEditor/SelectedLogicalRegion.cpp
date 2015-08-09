#include "SelectedLogicalRegion.h"

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