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

#ifndef _OLOTAPILAYER_H_
#define _OLOTAPILAYER_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <mutex>

#include "openxr/openxr.h"
#include "openxr/loader_interfaces.h"

#include "olotStructs.h"
#include "olotInstance.h"

class olotOcsClient;


/**
 * API layer class.
 */
class olotApiLayer{
public:
	/** Instance map. */
	typedef std::unordered_map<XrInstance,olotInstance::Ref> MapInstances;
	
	/** Session map. */
	typedef std::unordered_map<XrSession,olotInstance*> MapSessions;
	
	/** Space map. */
	typedef std::unordered_map<XrSpace,olotSpace> MapSpaces;
	
	/** Action set map. */
	typedef std::unordered_map<XrActionSet,olotInstance*> MapActionSets;
	
	/** Action map. */
	typedef std::unordered_map<XrAction,olotInstance*> MapActions;
	
	
	
private:
	std::string pLayerName;
	
	bool pSupportsEyeGazeTracking;
	bool pSupportsFacialTracking;
	
	MapInstances pInstances;
	MapSessions pSessions;
	MapSpaces pSpaces;
	MapActionSets pActionSets;
	MapActions pActions;
	
	std::shared_ptr<olotOcsClient> pOcsClient;
	
	std::ofstream pLogFile;



public:
	std::mutex mutexLog;
	

	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** Create API layer. */
	olotApiLayer();
	
	/** Clean up API layer. */
	~olotApiLayer();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** Global instance. */
	static olotApiLayer &Get();
	
	/** Negotiate. */
	XrResult Negotiate( const XrNegotiateLoaderInfo *loaderInfo,
		const char *layerName, XrNegotiateApiLayerRequest *apiLayerRequest );
	
	/** Layer name. */
	inline const std::string &GetLayerName() const{ return pLayerName; }
	
	/** Eye gaze tracking is supported. */
	inline bool GetSupportsEyeGazeTracking() const{ return pSupportsEyeGazeTracking; }
	
	/** Facial tracking is supported. */
	inline bool GetSupportsFacialTracking() const{ return pSupportsFacialTracking; }
	
	
	
	/** Instances. */
	inline MapInstances &GetInstances(){ return pInstances; }
	inline const MapInstances &GetInstances() const{ return pInstances; }
	
	/** Instance is present. */
	bool HasInstance( XrInstance instance ) const;
	
	/** Instance by handle or nullptr. */
	olotInstance::Ref GetInstance( XrInstance instance ) const;
	
	/** Add instance. */
	void AddInstance( const olotInstance::Ref &instance );
	
	/** Remove instance. */
	void RemoveInstance( XrInstance instance );
	
	
	
	/** Sessions. */
	inline MapSessions &GetSessions(){ return pSessions; }
	inline const MapSessions &GetSessions() const{ return pSessions; }
	
	/** Spaces. */
	inline MapSpaces &GetSpaces(){ return pSpaces; }
	inline const MapSpaces &GetSpaces() const{ return pSpaces; }
	
	/** Action sets. */
	inline MapActionSets &GetActionSets(){ return pActionSets; }
	inline const MapActionSets &GetActionSets() const{ return pActionSets; }
	
	/** Actions. */
	inline MapActions &GetActions(){ return pActions; }
	inline const MapActions &GetActions() const{ return pActions; }
	
	
	
	/** Acquire VIVE SDK. */
	olotOcsClient *AcquireOcsClient();
	
	/** Drop VIVE SDK. */
	void DropOcsClient();
	
	
	
	/** Base log stream. */
	std::ostream &baseLogStream();

	/** Log stream. */
	std::ostream &log();
	/*@}*/
};

#endif
