#ifndef __ELASTOS_DROID_INTERNAL_TELEPHONY_CAT_MENU_H__
#define __ELASTOS_DROID_INTERNAL_TELEPHONY_CAT_MENU_H__

#include "elastos/droid/ext/frameworkext.h"
#include "elastos/core/Object.h"

using Elastos::Droid::Graphics::IBitmap;
// import java.util.ArrayList;
using Elastos::Utility::IList;

namespace Elastos {
namespace Droid {
namespace Internal {
namespace Telephony {
namespace Cat {

/**
  * Container class for CAT menu (SET UP MENU, SELECT ITEM) parameters.
  *
  */
class Menu
    : public Object
    , public IMenu
    , public IParcelable
{
//private:
//    class InnerParcelableCreator
//        : public Object
//        , public Parcelable::Creator<Menu>
//    {
//    public:
//        InnerParcelableCreator(
//            /* [in] */ Menu* owner);
//
//        // @Override
//        CARAPI CreateFromParcel(
//            /* [in] */ IParcel* in,
//            /* [out] */ Menu** result);
//
//        // @Override
//        CARAPI NewArray(
//            /* [in] */ Int32 size,
//            /* [out] */ Menu[]** result);
//
//    private:
//        Menu* mOwner;
//    };

public:
    CAR_INTERFACE_DECL();

    Menu();

    CARAPI constructor();

    //// @Override
    //CARAPI DescribeContents(
    //    /* [out] */ Int32* result);

    // @Override
    CARAPI WriteToParcel(
        /* [in] */ IParcel* dest);
        ///* [in] */ Int32 flags);

    CARAPI ReadFromParcel(
        /* [in] */ IParcel* source);

private:
    CARAPI constructor(
        /* [in] */ IParcel* in);

public:
    AutoPtr<IList/*<Item>*/> items;
    AutoPtr<IList/*<TextAttribute>*/> titleAttrs;
    PresentationType presentationType;
    String title;
    AutoPtr<IBitmap> titleIcon;
    Int32 defaultItem;
    Boolean softKeyPreferred;
    Boolean helpAvailable;
    Boolean titleIconSelfExplanatory;
    Boolean itemsIconSelfExplanatory;
    //static const AutoPtr<IParcelable> AutoPtr< ::Creator<Menu> > CREATOR;
};

} // namespace Cat
} // namespace Telephony
} // namespace Internal
} // namespace Droid
} // namespace Elastos

#endif // __ELASTOS_DROID_INTERNAL_TELEPHONY_CAT_MENU_H__
