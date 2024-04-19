# OpenXR Layer OSC Client for Eye/Face tracking

Provides OpenXR API Layer receiving eye/face tracking data over OSC.
This layer provides these OpenXR Extensions:

- XR_EXT_eye_gaze_interaction
- XR_HTC_facial_tracking

# Limitations

For XR_EXT_eye_gaze_interaction only Local Space is supported.
If this is a problem with an application please create an issue on GitHub.

# Motivation

Applications like [https://github.com/SummerSigh/ProjectBabble/](Project Babble)
stream tracked data to an OSC compatible client. This client can be for
example [https://docs.vrcft.io/](VRCFaceTracking) to manipulate an avatar
inside a host application which is typically using OpenVR. Being able to
directly receive this data inside OpenXR allows to use this transparently
and in a standardized way. This OpenXR layer provides exactly this.

# Installation

TODO

# Enable/Disable

Open the SteamVR Settings Window. Switch on/off _API Layer OSC Eye/Face Tracking_.
Make sure only one API layer providing eye/face is enabled or the layers
can trample over each other causing strange results.

# Uninstalling

Open the Windows "Add/Remove Applications" app. Locate the application
_OpenXR Layer OCS Eye/Face Tracking_ and uninstall it.

# Support

In case of problems create an issue on GitHub or drop by the the
[Discord channel](https://discord.gg/Jeg62ns).

# Author

[DragonDreams GmbH](https://dragondreams.ch/)
