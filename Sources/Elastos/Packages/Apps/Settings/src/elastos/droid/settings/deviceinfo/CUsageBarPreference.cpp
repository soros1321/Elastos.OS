
#include "elastos/droid/settings/deviceinfo/CUsageBarPreference.h"
#include "../R.h"

using Elastos::Droid::Preference::IPreference;
using Elastos::Utility::CArrayList;
using Elastos::Utility::ICollections;
using Elastos::Utility::CCollections;

namespace Elastos {
namespace Droid {
namespace Settings {
namespace Deviceinfo {

CAR_OBJECT_IMPL(CUsageBarPreference)

CUsageBarPreference::CUsageBarPreference()
{}

CUsageBarPreference::~CUsageBarPreference()
{}

ECode CUsageBarPreference::constructor(
    /* [in] */ IContext* context)
{
    return constructor(context, NULL);
}

ECode CUsageBarPreference::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs)
{
    return constructor(context, attrs, 0);
}

ECode CUsageBarPreference::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyle)
{
    return constructor(context, attrs, defStyle, 0);
}

ECode CUsageBarPreference::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyleAttr,
    /* [in] */ Int32 defStyleRes)
{
    CArrayList::New((IList**)&mEntries);
    Preference::constructor(context, attrs, defStyleAttr, defStyleRes);
    return SetLayoutResource(R::layout::preference_memoryusage);
}

ECode CUsageBarPreference::AddEntry(
    /* [in] */ Int32 order,
    /* [in] */ Float percentage,
    /* [in] */ Int32 color)
{
    mEntries->Add((IObject*)CPercentageBarChart::CreateEntry(order, percentage, color));
    AutoPtr<ICollections> coll;
    CCollections::AcquireSingleton((ICollections**)&coll);
    coll->Sort(mEntries);
    return NOERROR;
}

ECode CUsageBarPreference::OnBindView(
    /* [in] */ IView* view)
{
    Preference::OnBindView(view);

    AutoPtr<IView> tmp;
    view->FindViewById(R::id::percentage_bar_chart, (IView**)&tmp);
    mChart = (CPercentageBarChart*)tmp.Get();
    mChart->SetEntries(ICollection::Probe(mEntries));
    return NOERROR;
}

ECode CUsageBarPreference::Commit()
{
    if (mChart != NULL) {
        mChart->Invalidate();
    }
    return NOERROR;
}

ECode CUsageBarPreference::Clear()
{
    return mEntries->Clear();
}

} // namespace Deviceinfo
} // namespace Settings
} // namespace Droid
} // namespace Elastos