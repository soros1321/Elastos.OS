//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include "elastos/droid/ext/frameworkext.h"
#include "elastos/droid/telecom/CVideoProviderBinder.h"

using Elastos::Droid::Os::IMessage;
using Elastos::Droid::Internal::Telecom::EIID_IIVideoProvider;
using Elastos::Core::CString;
using Elastos::Core::IInteger32;
using Elastos::Core::CInteger32;
using Elastos::Core::IFloat;
using Elastos::Core::CFloat;

namespace Elastos {
namespace Droid {
namespace Telecom {

CAR_OBJECT_IMPL(CVideoProviderBinder)

CAR_INTERFACE_IMPL(CVideoProviderBinder, Object, IIVideoProvider)

ECode CVideoProviderBinder::constructor(
    /* [in] */ IHandler* handler)
{
    mMessageHandler = handler;
    return NOERROR;
}

ECode CVideoProviderBinder::SetVideoCallback(
    /* [in] */ IBinder* videoCallbackBinder)
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_VIDEO_CALLBACK, videoCallbackBinder, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SetCamera(
    /* [in] */ const String& cameraId)
{
    AutoPtr<ICharSequence> pId;
    CString::New(cameraId, (ICharSequence**)&pId);
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_CAMERA, pId, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SetPreviewSurface(
    /* [in] */ ISurface* surface)
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_PREVIEW_SURFACE, (IInterface*)(IParcelable*)surface, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SetDisplaySurface(
    /* [in] */ ISurface* surface)
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_DISPLAY_SURFACE, (IInterface*)(IParcelable*)surface, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SetDeviceOrientation(
    /* [in] */ Int32 rotation)
{
    AutoPtr<IInteger32> pRotation;
    CInteger32::New(rotation, (IInteger32**)&pRotation);
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_DEVICE_ORIENTATION, pRotation, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SetZoom(
    /* [in] */ Float value)
{
    AutoPtr<IFloat> pVal;
    CFloat::New(value, (IFloat**)&pVal);
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_ZOOM, pVal, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SendSessionModifyRequest(
    /* [in] */ IVideoProfile* requestProfile)
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SEND_SESSION_MODIFY_REQUEST, requestProfile, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SendSessionModifyResponse(
    /* [in] */ IVideoProfile* responseProfile)
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SEND_SESSION_MODIFY_RESPONSE, responseProfile, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::RequestCameraCapabilities()
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_REQUEST_CAMERA_CAPABILITIES, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::RequestCallDataUsage()
{
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_REQUEST_CALL_DATA_USAGE, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

ECode CVideoProviderBinder::SetPauseImage(
    /* [in] */ const String& uri)
{
    AutoPtr<ICharSequence> pUri;
    CString::New(uri, (ICharSequence**)&pUri);
    AutoPtr<IMessage> msg;
    mMessageHandler->ObtainMessage(IConnectionVideoProvider::MSG_SET_PAUSE_IMAGE, pUri, (IMessage**)&msg);
    msg->SendToTarget();
    return NOERROR;
}

} // namespace Telecom
} // namespace Droid
} // namespace Elastos