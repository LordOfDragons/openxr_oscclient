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

#ifndef _OLOTEYEGAZETRACKER_H_
#define _OLOTEYEGAZETRACKER_H_

#include <memory>
#include <vector>

#include "openxr/openxr.h"
#include "olotOcsClient.h"
#include "olotStructs.h"

class olotInstance;


/**
 * Eye gaze tracker class.
 */
class olotEyeGazeTracker{
public:
	/** Reference. */
	typedef std::shared_ptr<olotEyeGazeTracker> Ref;
	
	/** Action list. */
	typedef std::vector<XrAction> ListActions;
	
	
	
private:
	olotInstance &pInstance;
	
	XrPath pPathPose;
	ListActions pActions;
	
	float pOcsValues[ olotOcsClient::EyeStateCount ];
	
	bool pActive;
	XrPosef pPose;
	
	olotOcsClient *pOcsClient;
	bool pEyeEngineStarted;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** Create eye gaze tracker. */
	olotEyeGazeTracker( olotInstance &instance );
	
	/** Clean up eye gaze tracker. */
	~olotEyeGazeTracker();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** Instance. */
	inline olotInstance &GetInstance() const{ return pInstance; }
	
	/** Pose path. */
	inline XrPath GetPathPose() const{ return pPathPose; }
	
	/** Matches action. */
	bool Matches( XrAction action, XrPath subactionPath ) const;
	
	/** xrSuggestInteractionProfileBindings. */
	XrResult SuggestInteractionProfileBindings(
		const XrInteractionProfileSuggestedBinding &suggestedBindings );
	
	/** xrGetActionStatePose. */
	XrResult GetActionStatePose( XrActionStatePose &state );
	
	/** xrLocateSpace. */
	XrResult LocateSpace( const olotSpace &space, XrSpace baseSpace,
		XrTime time, XrSpaceLocation *location );
	
	/** Log stream. */
	std::ostream &log();
	/*@}*/
	
	
	
private:
	void pCleanUp();
};

#endif
