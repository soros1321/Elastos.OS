
#include "elastos/droid/packages/systemui/CSomnambulator.h"
#include "elastos/droid/packages/systemui/Utils.h"
#include "Elastos.Droid.Service.h"
#include "R.h"

using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::CIntentShortcutIconResourceHelper;
using Elastos::Droid::Content::EIID_IContext;
using Elastos::Droid::Content::IIntentShortcutIconResourceHelper;
using Elastos::Droid::Content::IIntentShortcutIconResource;
using Elastos::Droid::Service::Dreams::CSandman;
using Elastos::Droid::Service::Dreams::ISandman;

namespace Elastos {
namespace Droid {
namespace Packages {
namespace SystemUI {

CAR_OBJECT_IMPL(CSomnambulator);
ECode CSomnambulator::constructor()
{
    return NOERROR;
}

ECode CSomnambulator::OnStart()
{
    Activity::OnStart();

    AutoPtr<IIntent> launchIntent;
    GetIntent((IIntent**)&launchIntent);
    String action;
    launchIntent->GetAction(&action);
    if (IIntent::ACTION_CREATE_SHORTCUT.Equals(action)) {
        AutoPtr<IIntent> shortcutIntent;
        AutoPtr<IClassInfo> info = Utils::GetClassInfo(String("CSomnambulator"));
        assert(info != NULL);
        ClassID id;
        info->GetId(&id);
        CIntent::New(this, id, (IIntent**)&shortcutIntent);
        shortcutIntent->SetFlags(IIntent::FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | IIntent::FLAG_ACTIVITY_NEW_TASK);
        AutoPtr<IIntent> resultIntent;
        CIntent::New((IIntent**)&resultIntent);
        AutoPtr<IIntentShortcutIconResourceHelper> helper;
        CIntentShortcutIconResourceHelper::AcquireSingleton((IIntentShortcutIconResourceHelper**)&helper);
        AutoPtr<IIntentShortcutIconResource> sr;
        helper->FromContext(this, R::mipmap::ic_launcher_dreams, (IIntentShortcutIconResource**)&sr);
        resultIntent->PutExtra(IIntent::EXTRA_SHORTCUT_ICON_RESOURCE, IParcelable::Probe(sr));
        resultIntent->PutExtra(IIntent::EXTRA_SHORTCUT_INTENT, IParcelable::Probe(shortcutIntent));
        String value;
        GetString(R::string::start_dreams, &value);
        resultIntent->PutExtra(IIntent::EXTRA_SHORTCUT_NAME, value);
        SetResult(RESULT_OK, resultIntent);
    }
    else {
        Boolean docked = FALSE;
        launchIntent->HasCategory(IIntent::CATEGORY_DESK_DOCK, &docked);
        AutoPtr<ISandman> s;
        CSandman::AcquireSingleton((ISandman**)&s);
        if (docked) {
            s->StartDreamWhenDockedIfAppropriate(THIS_PROBE(IContext));
        }
        else {
            s->StartDreamByUserRequest(THIS_PROBE(IContext));
        }
    }
    Finish();
    return NOERROR;
}

} // namespace SystemUI
} // namespace Packages
} // namespace Droid
} // namespace Elastos
