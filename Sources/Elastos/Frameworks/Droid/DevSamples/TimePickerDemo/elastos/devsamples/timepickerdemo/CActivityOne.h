
#ifndef __Elastos_DevSamples_TimePickerDemo_CActivityOne_H__
#define __Elastos_DevSamples_TimePickerDemo_CActivityOne_H__

#include <Elastos.Droid.Widget.h>
#include <elastos/droid/app/Activity.h>
#include "_Elastos_DevSamples_TimePickerDemo_CActivityOne.h"

using Elastos::Droid::App::Activity;
using Elastos::Droid::Os::IBundle;
using Elastos::Droid::Widget::ITimePicker;

namespace Elastos {
namespace DevSamples {
namespace TimePickerDemo {

CarClass(CActivityOne)
    , public Activity
{
public:
    CAR_OBJECT_DECL()

    CARAPI constructor();

protected:
    CARAPI OnCreate(
        /* [in] */ IBundle* savedInstanceState);

    CARAPI OnStart();

    CARAPI OnResume();

    CARAPI OnPause();

    CARAPI OnStop();

    CARAPI OnDestroy();

private:
    AutoPtr<ITimePicker> mTimePicker;
};

} // namespace TimePickerDemo
} // namespace DevSamples
} // namespace Elastos

#endif // __Elastos_DevSamples_TimePickerDemo_CActivityOne_H__
