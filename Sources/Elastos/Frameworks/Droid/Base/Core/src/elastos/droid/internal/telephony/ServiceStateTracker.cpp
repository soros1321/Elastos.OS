

#include <elastos/core/AutoLock.h>
#include <elastos/core/CoreUtils.h>
#include <elastos/core/IntegralToString.h>
#include <elastos/core/StringToIntegral.h>
#include "elastos/droid/internal/telephony/ServiceStateTracker.h"
#include "elastos/droid/R.h"
#include "elastos/droid/text/TextUtils.h"

using Elastos::Core::AutoLock;
using Elastos::Core::CoreUtils;
using Elastos::Core::IntegralToString;
using Elastos::Core::StringUtils;

using Elastos::Droid::Content::Res::IResources;
using Elastos::Droid::Internal::Telephony::Dataconnection::IDcTrackerBase;
using Elastos::Droid::Internal::Telephony::Uicc::IccCardApplicationStatus::IAppState;
using Elastos::Droid::Internal::Telephony::Uicc::IIccRecords;
using Elastos::Droid::Internal::Telephony::Uicc::IUiccCardApplication;
using Elastos::Droid::Internal::Telephony::Uicc::IUiccController;
using Elastos::Droid::Net::IConnectivityManager;
using Elastos::Droid::Net::INetworkInfo;
using Elastos::Droid::Os::IAsyncResult;
//using Elastos::Droid::Os::IHandler;
using Elastos::Droid::Os::IMessage;
using Elastos::Droid::Os::IRegistrant;
using Elastos::Droid::Os::IRegistrantList;
using Elastos::Droid::R;
using Elastos::Droid::Telephony::IServiceStateHelper;
using Elastos::Droid::Telephony::CServiceStateHelper;
using Elastos::Droid::Text::ITextUtils;
using Elastos::Droid::Text::TextUtils;
using Elastos::Droid::Utility::CPair;
using Elastos::Droid::Utility::IPair;


namespace Elastos {
namespace Droid {
namespace Internal {
namespace Telephony {

/**
 * {@hide}
 */
CAR_INTERFACE_IMPL(ServiceStateTracker, Handler, IServiceStateTracker)

const Boolean DBG = TRUE;
const Boolean VDBG = FALSE;
const String PROP_FORCE_ROAMING("telephony.test.forceRoaming");

/** Signal strength poll rate. */
const Int32 POLL_PERIOD_MILLIS = 20 * 1000;

/** GSM events */
const Int32 EVENT_RADIO_STATE_CHANGED               = 1;
const Int32 EVENT_NETWORK_STATE_CHANGED             = 2;
const Int32 EVENT_GET_SIGNAL_STRENGTH               = 3;
const Int32 EVENT_POLL_STATE_REGISTRATION           = 4;
const Int32 EVENT_POLL_STATE_GPRS                   = 5;
const Int32 EVENT_POLL_STATE_OPERATOR               = 6;
const Int32 EVENT_POLL_SIGNAL_STRENGTH              = 10;
const Int32 EVENT_NITZ_TIME                         = 11;
const Int32 EVENT_SIGNAL_STRENGTH_UPDATE            = 12;
const Int32 EVENT_RADIO_AVAILABLE                   = 13;
const Int32 EVENT_POLL_STATE_NETWORK_SELECTION_MODE = 14;
const Int32 EVENT_GET_LOC_DONE                      = 15;
const Int32 EVENT_SIM_RECORDS_LOADED                = 16;
const Int32 EVENT_SIM_READY                         = 17;
const Int32 EVENT_LOCATION_UPDATES_ENABLED          = 18;
const Int32 EVENT_GET_PREFERRED_NETWORK_TYPE        = 19;
const Int32 EVENT_SET_PREFERRED_NETWORK_TYPE        = 20;
const Int32 EVENT_RESET_PREFERRED_NETWORK_TYPE      = 21;
const Int32 EVENT_CHECK_REPORT_GPRS                 = 22;
const Int32 EVENT_RESTRICTED_STATE_CHANGED          = 23;

/** CDMA events */
const Int32 EVENT_POLL_STATE_REGISTRATION_CDMA      = 24;
const Int32 EVENT_POLL_STATE_OPERATOR_CDMA          = 25;
const Int32 EVENT_RUIM_READY                        = 26;
const Int32 EVENT_RUIM_RECORDS_LOADED               = 27;
const Int32 EVENT_POLL_SIGNAL_STRENGTH_CDMA         = 28;
const Int32 EVENT_GET_SIGNAL_STRENGTH_CDMA          = 29;
const Int32 EVENT_NETWORK_STATE_CHANGED_CDMA        = 30;
const Int32 EVENT_GET_LOC_DONE_CDMA                 = 31;
//const Int32 EVENT_UNUSED                            = 32;
const Int32 EVENT_NV_LOADED                         = 33;
const Int32 EVENT_POLL_STATE_CDMA_SUBSCRIPTION      = 34;
const Int32 EVENT_NV_READY                          = 35;
const Int32 EVENT_ERI_FILE_LOADED                   = 36;
const Int32 EVENT_OTA_PROVISION_STATUS_CHANGE       = 37;
const Int32 EVENT_SET_RADIO_POWER_OFF               = 38;
const Int32 EVENT_CDMA_SUBSCRIPTION_SOURCE_CHANGED  = 39;
const Int32 EVENT_CDMA_PRL_VERSION_CHANGED          = 40;
const Int32 EVENT_RADIO_ON                          = 41;
const Int32 EVENT_ICC_CHANGED                       = 42;
const Int32 EVENT_GET_CELL_INFO_LIST                = 43;
const Int32 EVENT_UNSOL_CELL_INFO_LIST              = 44;
const Int32 EVENT_CHANGE_IMS_STATE                  = 45;

const AutoPtr<ArrayOf<String> > ServiceStateTracker::GMT_COUNTRY_CODES = InitCOUNTCODE();

const String TIMEZONE_PROPERTY("persist.sys.timezone");

/** Reason for registration denial. */
static const String REGISTRATION_DENIED_GEN("General");
static const String REGISTRATION_DENIED_AUTH("Authentication Failure");

static const String ACTION_RADIO_OFF("android.intent.action.ACTION_RADIO_OFF");

const Int64 LAST_CELL_INFO_LIST_MAX_AGE_MS = 2000;

ServiceStateTracker::CellInfoResult::CellInfoResult()
{
    AutoPtr<IList> list;
    Object lockObj;
}

ServiceStateTracker::ServiceStateTracker()
    : mLastSignalStrength(NULL)
    , mUiccController(NULL)
    , mUiccApplcation(NULL)
    , mIccRecords(NULL)
    , mLastCellInfoList(NULL)
    , mIntentFilter(NULL)
    , mRadioOffIntent(NULL)
    , mDontPollSignalStrength(FALSE)
    , mPendingRadioPowerOffAfterDataOff(FALSE)
    , mPendingRadioPowerOffAfterDataOffTag(0)
    , mIwlanRatAvailable(FALSE)
    , mImsRegistrationOnOff(FALSE)
    , mAlarmSwitch(FALSE)
    , mPowerOffDelayNeed(TRUE)
    , mDeviceShuttingDown(FALSE)
{
    CServiceState::New((IServiceState**)&mSS);
    CServiceState::New((IServiceState**)&mNewSS);

    CSignalStrength::New((ISignalStrength**)&mSignalStrength);
    CRestrictedState::New((IRestrictedState**)&mRestrictedState);

    CRegistrantList::New((IRegistrantList**)&mRoamingOnRegistrants);
    CRegistrantList::New((IRegistrantList**)&mRoamingOffRegistrants);
    CRegistrantList::New((IRegistrantList**)&mAttachedRegistrants);
    CRegistrantList::New((IRegistrantList**)&mDetachedRegistrants);
    CRegistrantList::New((IRegistrantList**)&mDataRegStateOrRatChangedRegistrants);
    CRegistrantList::New((IRegistrantList**)&mNetworkAttachedRegistrants);
    CRegistrantList::New((IRegistrantList**)&mPsRestrictEnabledRegistrants);
    CRegistrantList::New((IRegistrantList**)&mPsRestrictDisabledRegistrants);
}

static CARAPI_(AutoPtr<ArrayOf<String> >) InitCOUNTCODE()
{
    AutoPtr<ArrayOf<String> > sArray = ArrayOf<String>::Alloc(20);

    (*sArray)[0] = String("bf"); // Burkina Faso
    (*sArray)[1] = String("ci"); // Cote d'Ivoire
    (*sArray)[2] = String("eh"); // Western Sahara
    (*sArray)[3] = String("fo"); // Faroe Islands, Denmark
    (*sArray)[4] = String("gb"); // United Kingdom of Great Britain and Northern Ireland
    (*sArray)[5] = String("gh"); // Ghana
    (*sArray)[6] = String("gm"); // Gambia
    (*sArray)[7] = String("gn"); // Guinea
    (*sArray)[8] = String("gw"); // Guinea Bissau
    (*sArray)[9] = String("ie"); // Ireland
    (*sArray)[10] = String("lr"); // Liberia
    (*sArray)[11] = String("is"); // Iceland
    (*sArray)[12] = String("ma"); // Morocco
    (*sArray)[13] = String("ml"); // Mali
    (*sArray)[14] = String("mr"); // Mauritania
    (*sArray)[15] = String("pt"); // Portugal
    (*sArray)[16] = String("sl"); // Sierra Leone
    (*sArray)[17] = String("sn"); // Senegal
    (*sArray)[18] = String("st"); // Sao Tome and Principe
    (*sArray)[19] = String("tg"); // Togo
    return sArray;
}

ECode ServiceStateTracker::constructor(IPhoneBase* phoneBase, ICommandsInterface* ci, ICellInfo* cellInfo)
{
    mPhoneBase = phoneBase;
    mCellInfo = cellInfo;
    mCi = ci;

    AutoPtr<IContext> context;
    mPhoneBase->GetContext((IContext**)&context);
    AutoPtr<IResources> res;
    context->GetResources((IResources**)&res);
    res->GetBoolean(R::bool::config_voice_capable, &mVoiceCapable);

    mUiccController = UiccController->GetInstance();
    mUiccController->RegisterForIccChanged(this, EVENT_ICC_CHANGED, NULL);
    mCi->SetOnSignalStrengthUpdate(this, EVENT_SIGNAL_STRENGTH_UPDATE, NULL);
    mCi->RegisterForCellInfoList(this, EVENT_UNSOL_CELL_INFO_LIST, NULL);

    String str;
    AutoPtr<IServiceStateHelper> ssHelper;
    CServiceStateHelper::AcquireSingleton((IServiceStateHelper**)&ssHelper);
    ssHelper->RilRadioTechnologyToString(IServiceState::RIL_RADIO_TECHNOLOGY_UNKNOWN, &str);
    return mPhoneBase->SetSystemProperty(ITelephonyProperties::PROPERTY_DATA_NETWORK_TYPE, str);
}

void ServiceStateTracker::RequestShutdown()
{
    if (mDeviceShuttingDown == TRUE) return;
    mDeviceShuttingDown = TRUE;
    mDesiredPowerState = FALSE;
    SetPowerStateToDesired();
}

ECode ServiceStateTracker::Dispose()
{
    mCi->UnSetOnSignalStrengthUpdate(this);
    mUiccController->UnregisterForIccChanged(this);
    mCi->UnregisterForCellInfoList(this);
    return NOERROR;
}

ECode ServiceStateTracker::GetDesiredPowerState(Boolean* result)
{
    VALIDATE_NOT_NULL(result)
    *result = mDesiredPowerState;
    return NOERROR;
}


ECode ServiceStateTracker::NotifySignalStrength(Boolean * result)
{
    VALIDATE_NOT_NULL(result)
    *result = FALSE;

    {
        AutoLock syncLock(mCellInfo);

        Boolean bEqual;
        if (IObject::Probe(mSignalStrength)->Equals(mLastSignalStrength, &bEqual), !bEqual) {
            FAIL_RETURN(mPhoneBase->NotifySignalStrength());
            *result = TRUE;
        }
    }
    return NOERROR;
}

/**
 * Notify all mDataConnectionRatChangeRegistrants using an
 * AsyncResult in msg.obj where AsyncResult#result contains the
 * new RAT as an Integer Object.
 */
void ServiceStateTracker::NotifyDataRegStateRilRadioTechnologyChanged()
{
    Int32 rat, drs;
    mSS->GetRilDataRadioTechnology(&rat);
    mSS->GetDataRegState(&drs);

    if (DBG) Log(String("notifyDataRegStateRilRadioTechnologyChanged: drs=") + \
            IntegralToString::ToString(drs) + String(" rat= ") + IntegralToString::ToString(rat));

    AutoPtr<IServiceStateHelper> ssHelper;
    String str;
    CServiceStateHelper::AcquireSingleton((IServiceStateHelper**)&ssHelper);
    ssHelper->RilRadioTechnologyToString(rat, &str);
    mPhoneBase->SetSystemProperty(ITelephonyProperties::PROPERTY_DATA_NETWORK_TYPE, str);

    AutoPtr<IPair> pair;
    CPair::New(CoreUtils::Convert(drs), CoreUtils::Convert(rat), (IPair**)&pair);
    mDataRegStateOrRatChangedRegistrants->NotifyResult(pair);
}

/**
 * Some operators have been known to report registration failure
 * data only devices, to fix that use DataRegState.
 */
void ServiceStateTracker::UseDataRegStateForDataOnlyDevices()
{
    if (mVoiceCapable == FALSE) {
//        if (DBG) {
//            Log("useDataRegStateForDataOnlyDevice: VoiceRegState=" + mNewSS->GetVoiceRegState()
//                + " DataRegState=" + mNewSS->GetDataRegState());
//        }
        // TODO: Consider not lying and instead have callers know the difference.
        Int32 state;
        mNewSS->GetDataRegState(&state);
        mNewSS->SetVoiceRegState(state);
    }
}

void ServiceStateTracker::UpdatePhoneObject()
{
    AutoPtr<IContext> context;
    mPhoneBase->GetContext((IContext**)&context);
    AutoPtr<IResources> res;
    context->GetResources((IResources**)&res);
    Boolean bResult;
    res->GetBoolean(R::bool::config_switch_phone_on_voice_reg_state_change, &bResult);
    if (bResult) {
        Int32 rt;
        mSS->GetRilVoiceRadioTechnology(&rt)
        mPhoneBase->UpdatePhoneObject(rt);
    }
}

/**
 * Registration point for combined roaming on
 * combined roaming is TRUE when roaming is TRUE and ONS differs SPN
 *
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForRoamingOn(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mRoamingOnRegistrants->Add(r);

    Boolean bRoaming;
    mSS->GetRoaming(&bRoaming);
    if (bRoaming) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForRoamingOn(IHandler* h)
{
    return mRoamingOnRegistrants->Remove(h);
}

/**
 * Registration point for combined roaming off
 * combined roaming is TRUE when roaming is TRUE and ONS differs SPN
 *
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForRoamingOff(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mRoamingOffRegistrants->Add(r);

    if (!mSS->GetRoaming()) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForRoamingOff(IHandler* h)
{
    return mRoamingOffRegistrants->Remove(h);
}

/**
 * Re-register network by toggling preferred network type.
 * This is a work-around to deregister and register network since there is
 * no ril api to set COPS=2 (deregister) only.
 *
 * @param onComplete is dispatched when this is complete.  it will be
 * an AsyncResult, and onComplete.obj.exception will be non-NULL
 * on failure.
 */
ECode ServiceStateTracker::ReRegisterNetwork(IMessage* onComplete)
{
    AutoPtr<IMessage> response;
    ObtainMessage(EVENT_GET_PREFERRED_NETWORK_TYPE, onComplete, (IMessage**)&response);
    return mCi->GetPreferredNetworkType(response);
}

ECode ServiceStateTracker::SetRadioPower(Boolean power)
{
    mDesiredPowerState = power;

    return SetPowerStateToDesired();
}

ECode ServiceStateTracker::EnableSingleLocationUpdate()
{
    if (mWantSingleLocationUpdate || mWantContinuousLocationUpdates) return NOERROR;

    mWantSingleLocationUpdate = TRUE;
    AutoPtr<IMessage> response;
    ObtainMessage(EVENT_LOCATION_UPDATES_ENABLED, (IMessage**)&response);
    return mCi->SetLocationUpdates(TRUE, response);
}

ECode ServiceStateTracker::EnableLocationUpdates()
{
    if (mWantSingleLocationUpdate || mWantContinuousLocationUpdates) return NOERROR;

    mWantContinuousLocationUpdates = TRUE;
    AutoPtr<IMessage> response;
    ObtainMessage(EVENT_LOCATION_UPDATES_ENABLED, (IMessage**)&response);
    return mCi->SetLocationUpdates(TRUE, response);
}

void ServiceStateTracker::DisableSingleLocationUpdate()
{
    mWantSingleLocationUpdate = FALSE;
    if (!mWantSingleLocationUpdate && !mWantContinuousLocationUpdates) {
        mCi->SetLocationUpdates(FALSE, NULL);
    }
}

ECode ServiceStateTracker::DisableLocationUpdates()
{
    mWantContinuousLocationUpdates = FALSE;
    if (!mWantSingleLocationUpdate && !mWantContinuousLocationUpdates) {
        mCi->SetLocationUpdates(FALSE, NULL);
    }
    return NOERROR;
}

//@Override
ECode ServiceStateTracker::HandleMessage(IMessage* msg)
{
    Int32 what, arg1, arg2;
    msg->GetWhat(&what);
    msg->GetArg1(&arg1);
    msg->GetArg2(&arg2);
    AutoPtr<IInterface> obj;
    msg->GetObj((IInterface**)&obj);

    switch (what) {
        case EVENT_SET_RADIO_POWER_OFF:
            {
                AutoLock syncLock(this);
                if (mPendingRadioPowerOffAfterDataOff &&
                        (arg1 == mPendingRadioPowerOffAfterDataOffTag)) {
                    if (DBG) Log(String("EVENT_SET_RADIO_OFF, turn radio off now."));
                    HangupAndPowerOff();
                    mPendingRadioPowerOffAfterDataOffTag += 1;
                    mPendingRadioPowerOffAfterDataOff = FALSE;
                } else {
                    Log(String("EVENT_SET_RADIO_OFF is stale arg1=") + IntegralToString(arg1) +
                            String("!= tag=") + IntegralToString(mPendingRadioPowerOffAfterDataOffTag));
                }
            }
            break;

        case EVENT_ICC_CHANGED:
            OnUpdateIccAvailability();
            break;

        case EVENT_GET_CELL_INFO_LIST: {
            IAsyncResult* ar = IAsyncResult::Probe(obj);
            CellInfoResult result = (CellInfoResult) ar.userObj;
            {
                AutoLock syncLock(result.lockObj);
                if (ar.exception != NULL) {
//                    Log("EVENT_GET_CELL_INFO_LIST: error ret NULL, e=" + ar.exception);
                    result.list = NULL;
                } else {
                    result.list = (List<CellInfo>) ar.result;

//                    if (VDBG) {
//                        Log("EVENT_GET_CELL_INFO_LIST: size=" + result.list->Size()
//                                + " list=" + result.list);
//                    }
                }
                mLastCellInfoListTime = SystemClock::GetElapsedRealtime();
                mLastCellInfoList = result.list;
                result.lockObj->Notify();
            }
            break;
        }

        case EVENT_UNSOL_CELL_INFO_LIST: {
            IAsyncResult* ar = IAsyncResult::Probe(obj);
            if (ar.exception != NULL) {
                Log("EVENT_UNSOL_CELL_INFO_LIST: error ignoring, e=" + ar.exception);
            } else {
                List<CellInfo> list = (List<CellInfo>) ar.result;
                if (DBG) {
                    Log("EVENT_UNSOL_CELL_INFO_LIST: size=" + list->Size()
                            + " list=" + list);
                }
                mLastCellInfoListTime = SystemClock::GetElapsedRealtime();
                mLastCellInfoList = list;
                mPhoneBase->NotifyCellInfo(list);
            }
            break;
        }

        default:
            Log(String("Unhandled message with number: ") + IntegralToString(what));
            break;
    }
}

/**
 * Registration point for transition into DataConnection attached.
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForDataConnectionAttached(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mAttachedRegistrants->Add(r);

    if (GetCurrentDataConnectionState() == IServiceState::STATE_IN_SERVICE) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForDataConnectionAttached(IHandler* h)
{
    mAttachedRegistrants->Remove(h);
    return NOERROR;
}

/**
 * Registration point for transition into DataConnection detached.
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForDataConnectionDetached(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mDetachedRegistrants->Add(r);

    if (GetCurrentDataConnectionState() != IServiceState::STATE_IN_SERVICE) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForDataConnectionDetached(IHandler* h)
{
    mDetachedRegistrants->Remove(h);
}

/**
 * Registration for DataConnection RIL Data Radio Technology changing. The
 * new radio technology will be returned AsyncResult#result as an Integer Object.
 * The AsyncResult will be in the notification Message#obj.
 *
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForDataRegStateOrRatChanged(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mDataRegStateOrRatChangedRegistrants->Add(r);
    NotifyDataRegStateRilRadioTechnologyChanged();
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForDataRegStateOrRatChanged(IHandler* h)
{
    mDataRegStateOrRatChangedRegistrants->Remove(h);
    return NOERROR;
}

/**
 * Registration point for transition into network attached.
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj in Message.obj
 */
ECode ServiceStateTracker::RegisterForNetworkAttached(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mNetworkAttachedRegistrants->Add(r);

    Int32 state;
    mSS->GetVoiceRegState(&state);
    if (state == IServiceState::STATE_IN_SERVICE) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForNetworkAttached(IHandler* h)
{
    mNetworkAttachedRegistrants->Remove(h);
    return NOERROR;
}

/**
 * Registration point for transition into packet service restricted zone.
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForPsRestrictedEnabled(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mPsRestrictEnabledRegistrants->Add(r);

    if (mRestrictedState->IsPsRestricted()) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForPsRestrictedEnabled(IHandler* h)
{
    mPsRestrictEnabledRegistrants->Remove(h);
    return NOERROR;
}

/**
 * Registration point for transition out of packet service restricted zone.
 * @param h handler to notify
 * @param what what code of message when delivered
 * @param obj placed in Message.obj
 */
ECode ServiceStateTracker::RegisterForPsRestrictedDisabled(IHandler* h, Int32 what, IInterface* obj)
{
    AutoPtr<IRegistrant> r;
    CRegistrant::New(h, what, obj, (IRegistrant**)&r);
    mPsRestrictDisabledRegistrants->Add(r);

    Boolean result;
    mRestrictedState->IsPsRestricted(&result);
    if (result) {
        r->NotifyRegistrant();
    }
    return NOERROR;
}

ECode ServiceStateTracker::UnregisterForPsRestrictedDisabled(IHandler* h)
{
    mPsRestrictDisabledRegistrants->Remove(h);
    return NOERROR;
}

/**
 * Clean up existing voice and data connection then turn off radio power.
 *
 * Hang up the existing voice calls to decrease call drop rate.
 */
ECode ServiceStateTracker::PowerOffRadioSafely(IDcTrackerBase* dcTracker)
{
    {
        AutoLock syncLock(this);
        if (!mPendingRadioPowerOffAfterDataOff) {
            // In some network, deactivate PDP connection cause releasing of RRC connection,
            // which MM/IMSI detaching request needs. Without this detaching, network can
            // not release the network resources previously attached.
            // So we are avoiding data detaching on these networks.
            AutoPtr<IContext> context;
            mPhoneBase->GetContext((IContext**)&context);
            AutoPtr<IResources> res;
            context->GetResources((IResources**)&res);
            AutoPtr< ArrayOf<String> > networkNotClearData;
            res->GetStringArray(R::array::networks_not_clear_data, (ArrayOf<String>**)&networkNotClearData);

            String currentNetwork;
            mSS->GetOperatorNumeric(&currentNetwork);

            if ((networkNotClearData != NULL) && (currentNetwork != NULL)) {
                Int32 arrayLen = networkNotClearData->GetLength();
                for (Int32 i = 0; i < arrayLen; i++) {
                    Boolean bEqual;
                    if (IObject::Probe(currentNetwork)->Equals(networkNotClearData[i], &bEqual), bEqual) {
                        // Don't clear data connection for this carrier
                        if (DBG)
                            Log("Not disconnecting data for " + currentNetwork);
                        HangupAndPowerOff();
                        return NOERROR;
                    }
                }
            }
            // To minimize race conditions we call cleanUpAllConnections on
            // both if else paths instead of before this isDisconnected test.
            Boolean result;
            if (dcTracker->IsDisconnected(&result), result) {
                // To minimize race conditions we do this after isDisconnected
                dcTracker->CleanUpAllConnections(Phone::REASON_RADIO_TURNED_OFF);
                if (DBG) Log("Data disconnected, turn off radio right away.");
                HangupAndPowerOff();
            } else {
                dcTracker->CleanUpAllConnections(Phone::REASON_RADIO_TURNED_OFF);

                AutoPtr<IMessageHelper> mHelper;
                AutoPtr<IMessage> msg;
                CMessageHelper::AcquireSingleton((IMessageHelper**)&mHelper);
                mHelper->Obtain(this, (IMessage**)&msg);
                msg->SetWhat(EVENT_SET_RADIO_POWER_OFF);
                msg->SetArg1(++mPendingRadioPowerOffAfterDataOffTag);

                if (SendMessageDelayed(msg, 30000, &result), result) {
                    if (DBG) Log("Wait upto 30s for data to disconnect, then turn off radio.");
                    mPendingRadioPowerOffAfterDataOff = TRUE;
                } else {
                    Log("Cannot send delayed Msg, turn off radio right away.");
                    HangupAndPowerOff();
                }
            }
        }
    }
    return NOERROR;
}

/**
 * process the pending request to turn radio off after data is disconnected
 *
 * return TRUE if there is pending request to process; FALSE otherwise.
 */
ECode ServiceStateTracker::ProcessPendingRadioPowerOffAfterDataOff(Boolean* result)
{
    VALIDATE_NOT_NULL(result)

    {
        AutoLock syncLock(this);
        if (mPendingRadioPowerOffAfterDataOff) {
            if (DBG) Log("Process pending request to turn radio off.");
            mPendingRadioPowerOffAfterDataOffTag += 1;
            HangupAndPowerOff();
            mPendingRadioPowerOffAfterDataOff = FALSE;
            *result = TRUE;
            return NOERROR;
        }
        *result = FALSE;
        return NOERROR;
    }
}

/**
 * send signal-strength-changed notification if changed Called both for
 * solicited and unsolicited signal strength updates
 *
 * @return TRUE if the signal strength changed and a notification was sent.
 */
Boolean ServiceStateTracker::OnSignalStrengthResult(IAsyncResult* ar, Boolean isGsm)
{
    SignalStrength oldSignalStrength = mSignalStrength;

    // This signal is used for both voice and data radio signal so parse
    // all fields

    if ((ar.exception == NULL) && (ar.result != NULL)) {
        mSignalStrength = (SignalStrength) ar.result;
        mSignalStrength->ValidateInput();
        mSignalStrength->SetGsm(isGsm);
    } else {
        Log("OnSignalStrengthResult() Exception from RIL : " + ar.exception);
        mSignalStrength = new SignalStrength(isGsm);
    }

    return NotifySignalStrength();
}

/** Cancel a Pending (if any) PollState() operation */
void ServiceStateTracker::CancelPollState()
{
    // This will effectively cancel the rest of the poll requests.
    mPollingContext = ArrayOf<Int32>::Alloc(1);
}

/**
 * Return TRUE if time zone needs fixing.
 *
 * @param phoneBase
 * @param operatorNumeric
 * @param prevOperatorNumeric
 * @param needToFixTimeZone
 * @return TRUE if time zone needs to be fixed
 */
Boolean ServiceStateTracker::ShouldFixTimeZoneNow(IPhoneBase* phoneBase, String operatorNumeric,
        String prevOperatorNumeric, Boolean needToFixTimeZone)
{
    // Return FALSE if the mcc isn't valid as we don't know where we are.
    // Return TRUE if we have an IccCard and the mcc changed or we
    // need to fix it because when the NITZ time came in we didn't
    // know the country code.

    // if mcc is invalid then we'll return FALSE
    Int32 mcc;
    String subStr = operatorNumeric->Substring(0, 3);
    if (FAILED(StringToIntegral::Parse(subStr, &mcc)) {
        return FALSE;
    }

    // if prevMcc is invalid will make it different from mcc
    // so we'll return TRUE if the card exists.
    Int32 prevMcc;
    subStr = prevOperatorNumeric->Substring(0, 3);
    if (FAILED(StringToIntegral::Parse(subStr, &prevMcc)) {
        prevMcc = mcc + 1;
    }

    // Determine if the Icc card exists
    Boolean iccCardExist = FALSE;
    if (mUiccApplcation != NULL) {
        AppState aState;
        mUiccApplcation->GetState(&aState);
        iccCardExist = aState != AppState::APPSTATE_UNKNOWN;
    }

    // Determine retVal
    Boolean retVal = ((iccCardExist && (mcc != prevMcc)) || needToFixTimeZone);
    if (DBG) {
        AutoPtr<ISystem> sys;
        Elastos::Core::CSystem::AcquireSingleton((ISystem**)&sys);
        sys->GetCurrentTimeMillis(&ctm);
        Log("shouldFixTimeZoneNow: retVal=" + retVal +
                " iccCardExist=" + iccCardExist +
                " operatorNumeric=" + operatorNumeric + " mcc=" + mcc +
                " prevOperatorNumeric=" + prevOperatorNumeric + " prevMcc=" + prevMcc +
                " needToFixTimeZone=" + needToFixTimeZone +
                " ltod=" + TimeUtils->LogTimeOfDay(ctm));
    }
    return retVal;
}

ECode ServiceStateTracker::GetSystemProperty(String property, String defValue, String* result)
{
    AutoPtr<ITelephonyManager> tManager;
    CTelephonyManager::AcquireSingleton((ITelephonyManager**)&tManager);
    Int64 subId;
    mPhoneBase->GetSubId(&subId);
    return tManager->GetTelephonyProperty(property, subId, defValue, result);
}

/**
 * @return all available cell information or NULL if none.
 */
ECode ServiceStateTracker::GetAllCellInfo(IList** result)
{
    VALIDATE_NOT_NULL(result)

    CellInfoResult result = new CellInfoResult();
    if (VDBG) Log(String("SST->GetAllCellInfo(): E"));

    Int32 ver;
    mCi->GetRilVersion(&ver);
    if (ver >= 8) {
        if (IsCallerOnDifferentThread()) {
            if ((SystemClock::GetElapsedRealtime() - mLastCellInfoListTime)
                    > LAST_CELL_INFO_LIST_MAX_AGE_MS) {
                AutoPtr<IMessage> msg;
                ObtainMessage(EVENT_GET_CELL_INFO_LIST, result, (IMessage**)&msg);
                {
                    AutoLock syncLock(result.lockObj);
                    result.list = NULL;
                    mCi->GetCellInfoList(msg);
//                    try {
                        result.lockObj->Wait(5000);
//                    } Catch (InterruptedException e) {
//                        e->PrintStackTrace();
//                    }
                }
            } else {
                if (DBG) Log("SST->GetAllCellInfo(): return last, back to back calls");
                result.list = mLastCellInfoList;
            }
        } else {
            if (DBG) Log(String("SST->GetAllCellInfo(): return last, same thread can't block"));
            result.list = mLastCellInfoList;
        }
    } else {
        if (DBG) Log(String("SST->GetAllCellInfo(): not implemented"));
        result.list = NULL;
    }

    {
        AutoLock syncLock(result.lockObj);
        if (result.list != NULL) {
            if (DBG) Log("SST->GetAllCellInfo(): X size=" + result.list->Size()
                    + " list=" + result.list);
            *result = result.list;
            return NOERROR;
        } else {
            if (DBG) Log(String("SST->GetAllCellInfo(): X size=0 list=NULL"));

            *result = NULL;
            return NOERROR;
        }
    }
}

/**
 * @return signal strength
 */
ECode ServiceStateTracker::GetSignalStrength(ISignalStrength** result)
{
    VALIDATE_NOT_NULL(result)

    {
        AutoLock syncLock(mCellInfo);
        *result = mSignalStrength;
        REFCOUNT_ADD(*result)
        return NOERROR;
    }
}

ECode ServiceStateTracker::Dump(IFileDescriptor* fd, IPrintWriter* pw, ArrayOf<String>* args)
{
    pw->Println(String("ServiceStateTracker:"));
    pw->Println(String(" mSS=") + Object::ToString(mSS));
    pw->Println(String(" mNewSS=") + Object::ToString(mNewSS));
    pw->Println(String(" mCellInfo=") + Object::ToString(mCellInfo));
    pw->Println(String(" mRestrictedState=") + Object::ToString(mRestrictedState));
    pw->Println(String(" mPollingContext=") + Object::ToString(mPollingContext));
    pw->Println(String(" mDesiredPowerState=") + StringUtils::BooleanToString(mDesiredPowerState));
    pw->Println(String(" mDontPollSignalStrength=") + StringUtils::BooleanToString(mDontPollSignalStrength));
    pw->Println(String(" mPendingRadioPowerOffAfterDataOff=") + StringUtils::BooleanToString(mPendingRadioPowerOffAfterDataOff));
    pw->Println(String(" mPendingRadioPowerOffAfterDataOffTag=") + IntegralToString::ToString(mPendingRadioPowerOffAfterDataOffTag));
    return NOERROR;
}

/**
 * Verifies the current thread is the same as the thread originally
 * used in the initialization of this instance. Throws RuntimeException
 * if not.
 *
 * @exception RuntimeException if the current thread is not
 * the thread that originally obtained this PhoneBase instance.
 */
ECode ServiceStateTracker::CheckCorrectThread()
{
    AutoPtr<IThread> looperThread;
    AutoPtr<ILooper> handlerLooper;
    GetLooper((ILooper**)&handlerLooper);
    handlerLooper->GetThread((IThread**)&looperThread);
    if (!CObject::Compare(Thread::GetCurrentThread(), looperThread.Get()) {
        return E_RUNTIME_EXCEPTION
    }
    return NOERROR;
}

Boolean ServiceStateTracker::IsCallerOnDifferentThread()
{
    AutoPtr<IThread> looperThread;
    AutoPtr<ILooper> handlerLooper;
    GetLooper((ILooper**)&handlerLooper);
    handlerLooper->GetThread((IThread**)&looperThread);
    Boolean value = !CObject::Compare(Thread::GetCurrentThread(), looperThread.Get());
    if (VDBG) Log(String("isCallerOnDifferentThread: ") + StringUtils::BooleanToString(value));
    return value;
}

void ServiceStateTracker::UpdateCarrierMccMncConfiguration(String newOp, String oldOp, IContext* context)
{
    // if we have a change in operator, notify Wifi (even to/from none)
    if (((newOp == NULL) && (TextUtils::IsEmpty(oldOp) == FALSE)) ||
            ((newOp != NULL) && (newOp->Equals(oldOp) == FALSE))) {
        Log(String("update mccmnc=") + newOp + String(" fromServiceState=TRUE"));
        AutoPtr<IMccTableHelper> mccTableHelper;
        CMccTableHelper::AcquireSingleton((IMccTableHelper**)&mccTableHelper);
        mccTableHelper->UpdateMccMncConfiguration(context, newOp, TRUE);
    }
}

Boolean ServiceStateTracker::IsIwlanFeatureAvailable()
{
    AutoPtr<IContext> context;
    mPhoneBase->GetContext((IContext**)&context);
    AutoPtr<IResources> res;
    context->GetResources((IResources**)&res);
    Boolean iwlanAvailable;
    res->GetBoolean(R::bool::config_feature_iwlan_enabled, &iwlanAvailable);

    Log(String("Iwlan feature available = ") + StringUtils::BooleanToString(iwlanAvailable));
    return iwlanAvailable;
}

/* Consider the below usecase.
 * 1. WQE and IWLAN features are enabled and hence device can have wifi and mobile
 * connectivity simultaneously.
 * 2. Current available RATs are WWAN and IWLAN, both co-exists.
 * 3. RIL informs preferred RAT as WWAN. Telephony could have default and
 * non-default PDP activated. Since IWLAN is also available, non-default pdp
 * would be over IWLAN.
 * 4. WWAN goes to OOS.
 * 5. RIL informs that default PDP is Lost(unsol_data_call_list).
 * 6. Telephony attempts to retry the default APN context.
 * 7. RIL informs current preferred RAT is IWLAN.
 * 8. Telephony marks default/ia APN as "not available".
 * 9. Upon retry timer expiration, telephony does not bringup default APN
 * since is marked as unavailable. DC object moved to IDLE state.
 * 10 Later WWAN gets back in service.
 * 11. RIL informs preferred RAT as WWAN. So the RAT transition was as
 * follow.
 * IWLAN(Attached) -> WWAN(Attached)
 * 12. There is no trigger for telephony to initiate data call on
 * connnectable ApnContext after WWAN gets in service.
 * 13 Below method detects the transition from IWLAN to WWAN in attached
 * state and informs telephony that we are in WWAN attached state.
 * 14. Telephony would look into all the connectable APNs are would trigger
 * data call based on prevailing conditions.
 *
 */
void ServiceStateTracker::ProcessIwlanToWwanTransition(IServiceState* ss)
{
    // Wifi Connected(iwlan feature on) AND a Valid(non-wlan) RAT present
    // AND attached AND previous RAT was iwlan.
    //
    // Notify that we are attached so that we can setup connectable
    // APNs.
    Int32 rat, state;
    ss->GetRilDataRadioTechnology(&rat);
    ss->GetDataRegState(&state);

    if (IsIwlanFeatureAvailable() &&
            (rat != IServiceState::RIL_RADIO_TECHNOLOGY_IWLAN) &&
            (rat != IServiceState::RIL_RADIO_TECHNOLOGY_UNKNOWN) &&
            (state == IServiceState::STATE_IN_SERVICE) &&
            (mIwlanRatAvailable == TRUE)) {

        Log(String("pollStateDone: Wifi connected and moved out of iwlan and wwan is attached."));
        mAttachedRegistrants->NotifyRegistrants();
    }
}

} // namespace Telephony
} // namespace Internal
} // namespace Droid
} // namespace Elastos
