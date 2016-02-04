
#ifndef __ELASTOS_DROID_SERVER_PM_CPACKAGEINSTALLOBSERVER2_H__
#define __ELASTOS_DROID_SERVER_PM_CPACKAGEINSTALLOBSERVER2_H__

#include "_Elastos_Droid_Server_Pm_CPackageInstallObserver2.h"
#include "elastos/droid/server/pm/CPackageManagerService.h"

using Elastos::Droid::Os::IBundle;
using Elastos::Droid::Os::IBinder;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Content::Pm::IIPackageInstallObserver2;
using Elastos::Droid::Content::Pm::IIPackageMoveObserver;

namespace Elastos {
namespace Droid {
namespace Server {
namespace Pm {

CarClass(CPackageInstallObserver2)
    , public Object
    , public IIPackageInstallObserver2
    , public IBinder
{
public:
    CAR_INTERFACE_DECL()

    CAR_OBJECT_DECL()

    CARAPI constructor(
        /* [in] */ IIPackageManager* host,
        /* [in] */ IIPackageMoveObserver* observer,
        /* [in] */ const String& packageName);

    CARAPI OnUserActionRequired(
        /* [in] */ IIntent* intent);

    CARAPI OnPackageInstalled(
        /* [in] */ const String& basePackageName,
        /* [in] */ Int32 returnCode,
        /* [in] */ const String& msg,
        /* [in] */ IBundle* extras);

    CARAPI ToString(
        /* [out] */ String* str);

private:
    CPackageManagerService* mHost;
    AutoPtr<IIPackageMoveObserver> mObserver;
    String mPackageName;
};

} // namespace Pm
} // namespace Server
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_SERVER_PM_CPACKAGEINSTALLOBSERVER2_H__
