/**
 * MIT License
 * 
 * Copyright (c) 2024 DragonDreams (info@dragondreams.ch)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _OLOTINSTANCE_H_
#define _OLOTINSTANCE_H_

#include <string>
#include <memory>
#include <vector>

#include "openxr/openxr.h"

#include "olotEyeGazeTracker.h"
#include "olotFacialTracker.h"
#include "olotStructs.h"


/**
 * Instance class.
 */
class olotInstance{
public:
	/** Reference. */
	typedef std::shared_ptr<olotInstance> Ref;
	
	/** Facial trackers map. */
	typedef std::vector<olotFacialTracker::Ref> ListFacialTrackers;
	
	
	
private:
	const int pId;
	static int pNextId;
	
	const XrInstance pInstance;
	
	bool pEnableEyeGaze;
	bool pEnableFacial;
	
	PFN_xrGetInstanceProcAddr const pNextXrGetInstanceProcAddr;
	
	PFN_xrStringToPath pXrStringToPath;
	
	PFN_xrGetSystemProperties pNextXrGetSystemProperties;
	PFN_xrSuggestInteractionProfileBindings pNextXrSuggestInteractionProfileBindings;
	PFN_xrDestroyInstance pNextXrDestroyInstance;
	PFN_xrCreateSession pNextXrCreateSession;
	PFN_xrDestroySession pNextXrDestroySession;
	PFN_xrGetActionStatePose pNextXrGetActionStatePose;
	PFN_xrLocateSpace pNextXrLocateSpace;
	PFN_xrCreateActionSpace pNextXrCreateActionSpace;
	PFN_xrCreateReferenceSpace pNextXrCreateReferenceSpace;
	PFN_xrDestroySpace pNextXrDestroySpace;
	PFN_xrCreateActionSet pNextXrCreateActionSet;
	PFN_xrDestroyActionSet pNextXrDestroyActionSet;
	PFN_xrCreateAction pNextXrCreateAction;
	PFN_xrDestroyAction pNextXrDestroyAction;
	
	XrPath pPathProfileEyeGaze;
	
	olotEyeGazeTracker::Ref pEyeGazeTracker;
	ListFacialTrackers pFacialTrackers;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** Create instance. */
	olotInstance( PFN_xrGetInstanceProcAddr nextXrGetInstanceProcAddr,
		const XrInstanceCreateInfo &info, XrInstance instance );
	
	/** Clean up instance. */
	~olotInstance();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** Id. */
	inline int GetId() const{ return pId; }
	
	/** Instance. */
	inline XrInstance GetInstance() const{ return pInstance; }
	
	/** Get function. */
	XrResult GetInstanceProcAddr( const std::string &name, PFN_xrVoidFunction *function ) const;
	
	/** xrGetSystemProperties. */
	XrResult GetSystemProperties( XrSystemId systemId, XrSystemProperties *properties );
	
	/** xrSuggestInteractionProfileBindings. */
	XrResult SuggestInteractionProfileBindings(
		const XrInteractionProfileSuggestedBinding *suggestedBindings);
	
	/** xrDestroyInstance. */
	XrResult DestroyInstance();
	
	/** xrCreateSession. */
	XrResult CreateSession( const XrSessionCreateInfo *createInfo, XrSession *session );
	
	/** xrDestroySession. */
	XrResult DestroySession( XrSession session );
	
	/** xrCreateActionSet. */
	XrResult CreateActionSet( const XrActionSetCreateInfo *createInfo, XrActionSet *actionSet );
	
	/** xrCreateActionSet. */
	XrResult DestroyActionSet( XrActionSet actionSet );
	
	/** xrCreateAction. */
	XrResult CreateAction( XrActionSet actionSet,
		const XrActionCreateInfo *createInfo, XrAction *action );
	
	/** xrCreateAction. */
	XrResult DestroyAction( XrAction action );
	
	/** xrCreateActionSpace. */
	XrResult CreateActionSpace( XrSession session,
		const XrActionSpaceCreateInfo *createInfo, XrSpace *space );
	
	/** xrCreateReferenceSpace. */
	XrResult CreateReferenceSpace( XrSession session,
		const XrReferenceSpaceCreateInfo *createInfo, XrSpace *space );
	
	/** xrDestroyAction. */
	XrResult DestroySpace( XrSpace space );
	
	/** xrGetActionStatePose. */
	XrResult GetActionStatePose( XrSession session,
		const XrActionStateGetInfo *getInfo, XrActionStatePose *state );
	
	/** xrLocateSpace. */
	XrResult LocateSpace( const olotSpace &space, XrSpace baseSpace,
		XrTime time, XrSpaceLocation *location );
	
	/** xrCreateFacialTrackerHTC. */
	XrResult CreateFacialTracker( XrSession session,
		const XrFacialTrackerCreateInfoHTC *createInfo, XrFacialTrackerHTC *facialTracker );
	
	/** xrDestroyFacialTrackerHTC. */
	XrResult DestroyFacialTracker( olotFacialTracker *facialTracker );
	
	
	
	/** Get XrPath for string. */
	XrPath GetXrPathFor( const std::string &path ) const;
	
	/** Eye gazer tracker. */
	inline olotEyeGazeTracker::Ref &GetEyeGazeTracker(){ return pEyeGazeTracker; }
	inline const olotEyeGazeTracker::Ref &GetEyeGazeTracker() const{ return pEyeGazeTracker; }
	
	/** Facial tracker. */
	inline ListFacialTrackers &GetFacialTrackers(){ return pFacialTrackers; }
	inline const ListFacialTrackers &GetFacialTrackers() const{ return pFacialTrackers; }
	
	/** Log stream. */
	std::ostream &log();
	/*@}*/
	
	
	
private:
	void pCleanUp();
};

#endif
