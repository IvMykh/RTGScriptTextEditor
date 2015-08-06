#include "SelectedLogicalRegion.h"


SelectedTextRegion::SelectedTextRegion(const int lu, const int ll, const int ld, const int lr) :
	logicUp_(lu),
	logicLeft_(ll),
	logicDown(ld),
	logicRight_(lr)
{
}