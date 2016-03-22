
#include <Elastos.CoreLibrary.IO.h>
#include "Elastos.Droid.Content.h"
#include "elastos/droid/ext/frameworkdef.h"
#include "elastos/droid/os/RecoverySystem.h"
#include "elastos/droid/os/CRecoverySystem.h"

namespace Elastos {
namespace Droid {
namespace Os {

CAR_INTERFACE_IMPL(CRecoverySystem, Singleton, IRecoverySystem)

CAR_SINGLETON_IMPL(CRecoverySystem)

ECode CRecoverySystem::VerifyPackage(
    /* [in] */ IFile* packageFile,
    /* [in] */ IRecoverySystemProgressListener* listener,
    /* [in] */ IFile* deviceCertsZipFile)
{
    return RecoverySystem::VerifyPackage(packageFile, listener, deviceCertsZipFile);
}

ECode CRecoverySystem::InstallPackage(
    /* [in] */ IContext* ctx,
    /* [in] */ IFile* packageFile)
{
    return RecoverySystem::InstallPackage(ctx, packageFile);
}

ECode CRecoverySystem::RebootWipeUserData(
    /* [in] */ IContext* context)
{
    return RecoverySystem::RebootWipeUserData(context);
}

ECode CRecoverySystem::RebootWipeUserData(
    /* [in] */ IContext* context,
    /* [in] */ const String& reason)
{
    return RecoverySystem::RebootWipeUserData(context, reason);
}

ECode CRecoverySystem::RebootWipeUserData(
    /* [in] */ IContext* context,
    /* [in] */ Boolean shutdown)
{
    return RecoverySystem::RebootWipeUserData(context, shutdown);
}

ECode CRecoverySystem::RebootWipeUserData(
    /* [in] */ IContext* context,
    /* [in] */ Boolean shutdown,
    /* [in] */ const String& reason,
    /* [in] */ Boolean wipeMedia)
{
    return RecoverySystem::RebootWipeUserData(context, shutdown, reason, wipeMedia); 
}

ECode CRecoverySystem::RebootWipeCache(
    /* [in] */ IContext* context)
{
    return RecoverySystem::RebootWipeCache(context);
}

/** {@hide} */
ECode CRecoverySystem::RebootWipeCache(
    /* [in] */ IContext* context,
    /* [in] */ const String& reason)
{
    return RecoverySystem::RebootWipeCache(context, reason);
}

ECode CRecoverySystem::HandleAftermath(
    /* [out] */ String* result)
{
    VALIDATE_NOT_NULL(result)

    *result = RecoverySystem::HandleAftermath();
    return NOERROR;
}

} // namespace Os
} // namespace Droid
} // namespace Elastos
