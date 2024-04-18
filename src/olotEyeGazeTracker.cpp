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

#include <string.h>
#include <math.h>

#include "olotEyeGazeTracker.h"
#include "olotInstance.h"
#include "olotInstance.h"
#include "olotApiLayer.h"
#include "olotOcsClient.h"
#include "math/olotQuaternion.h"
#include "exceptions/exceptions.h"


// class olotEyeGazeTracker
/////////////////////////////

olotEyeGazeTracker::olotEyeGazeTracker( olotInstance &instance ) :
pInstance( instance ),
pPathPose( XR_NULL_PATH ),
pActive( false ),
pOcsClient( nullptr ),
pEyeEngineStarted( false )
{
	int i;
	for( i=0; i<olotOcsClient::EyeStateCount; i++ ){
		pOcsValues[ i ] = 0.0f;
	}
	
	try{
		pPathPose = instance.GetXrPathFor( "/user/eyes_ext/input/gaze_ext/pose" );
		
		memset( &pPose, 0, sizeof( pPose ) );
		pPose.orientation.w = 1.0f;
		
		pOcsClient = olotApiLayer::Get().AcquireOcsClient();
		pEyeEngineStarted = true;
		
	}catch( const olotException & ){
		pCleanUp();
		throw;
	}
}

olotEyeGazeTracker::~olotEyeGazeTracker(){
	pCleanUp();
}



// Management
///////////////

bool olotEyeGazeTracker::Matches( XrAction action, XrPath /*subactionPath*/ ) const{
	ListActions::const_iterator iter;
	for( iter = pActions.cbegin(); iter != pActions.cend(); iter++ ){
		if( *iter == action ){
			// TODO check subactionPath
			return true;
		}
	}
	return false;
}

XrResult olotEyeGazeTracker::SuggestInteractionProfileBindings(
const XrInteractionProfileSuggestedBinding &suggestedBindings ){
	{
	const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
	log() << "SuggestInteractionProfileBindings" << std::endl;
	}
	
	// validate
	if( suggestedBindings.countSuggestedBindings > 0 ){
		OLOTASSERT_NOTNULL( suggestedBindings.suggestedBindings, XR_ERROR_VALIDATION_FAILURE )
	}
	
	uint32_t i;
	for( i=0; i<suggestedBindings.countSuggestedBindings; i++ ){
		OLOTASSERT_TRUE( suggestedBindings.suggestedBindings[ i ].binding == pPathPose, XR_ERROR_VALIDATION_FAILURE )
	}
	
	// clear
	pActions.clear();
	for( i=0; i<suggestedBindings.countSuggestedBindings; i++ ){
		pActions.push_back( suggestedBindings.suggestedBindings[ i ].action );
	}
	
	return XR_SUCCESS;
}

static inline float clamp( float value ){
	return std::max( std::min( value, 1.0f ), 0.0f );
}

static inline float linearStep( float value, float from, float to ){
	return clamp( ( value - from ) / ( to - from ) );
}

static inline float linearStep( float value, float from, float to, float mapFrom, float mapTo ){
	return linearStep( value, from, to ) * ( mapTo - mapFrom ) + mapFrom;
}

static const float onePi = 3.14159265f / 180.0f;

XrResult olotEyeGazeTracker::GetActionStatePose( XrActionStatePose &state ){
	try{
		pOcsClient->GetEyeStateValues( pOcsValues, olotOcsClient::EyeStateCount );
		
		const float eyeRightX = pOcsValues[ olotOcsClient::eesRightEyeX ];
		const float eyeLeftX = pOcsValues[ olotOcsClient::eesLeftEyeX ];
		const float eyesY = pOcsValues[ olotOcsClient::eesEyesY ];
		
		const float maxRotX = onePi * 45.0f;
		const float maxRotY = onePi * 30.0f;
		
		const float rotHorz = linearStep( ( eyeRightX + eyeLeftX ) / 2.0f, -1.0f, 1.0f, maxRotX, -maxRotX );
		const float rotVert = linearStep( eyesY, -1.0f, 1.0f, -maxRotY, maxRotY );
		
		// store position. since we do not know the origin we assume 0
		// x: positive to the right
		// y: positive upwards
		// z: positive backwards
		pPose.position.x = 0.0f;
		pPose.position.y = 0.0f;
		pPose.position.z = 0.0f;
		
		// calculate orientation matching direction. we use Drag[en]gine quaternion
		// code for this which is fine since quaternions work across coordinate systems
		const olotQuaternion orientation( olotQuaternion::CreateFromEuler( rotVert, rotHorz, 0.0f ) );
		
		pPose.orientation.x = orientation.x;
		pPose.orientation.y = orientation.y;
		pPose.orientation.z = orientation.z;
		pPose.orientation.w = orientation.w;
		
		pActive = true;
		
	}catch( const olotException & ){
		pActive = false;
	}
	
	state.type = XR_TYPE_ACTION_STATE_POSE;
	state.next = nullptr;
	state.isActive = pActive ? XR_TRUE : XR_FALSE;
	
	return XR_SUCCESS;
}

XrResult olotEyeGazeTracker::LocateSpace( const olotSpace &space,
XrSpace /*baseSpace*/, XrTime /*time*/, XrSpaceLocation *location ){
	location->locationFlags = 0;
	
	if( pActive ){
		location->pose = pPose;
		location->locationFlags = XR_SPACE_LOCATION_POSITION_VALID_BIT
			| XR_SPACE_LOCATION_POSITION_TRACKED_BIT
			| XR_SPACE_LOCATION_ORIENTATION_VALID_BIT
			| XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
		
	}else{
		memset( &location->pose, 0, sizeof( location->pose ) );
		location->pose.orientation.w = 1.0f;
	}
	
	void *next = location->next;
	
	while( next ){
		const XrBaseOutStructure &bos = *( ( XrBaseOutStructure* )next );
		
		switch( bos.type ){
		case XR_TYPE_SPACE_VELOCITY:{
			XrSpaceVelocity &sv = *( ( XrSpaceVelocity* )next );
			sv.velocityFlags = 0;
			
			if( pActive ){
				sv.linearVelocity = { 0.0f, 0.0f, 0.0f };
				sv.angularVelocity = { 0.0f, 0.0f, 0.0f };
				sv.velocityFlags = XR_SPACE_VELOCITY_LINEAR_VALID_BIT
					| XR_SPACE_VELOCITY_ANGULAR_VALID_BIT;
			}
			}break;
			
		default:
			break;
		}
		
		next = bos.next;
	}
	
	return XR_SUCCESS;
}

std::ostream &olotEyeGazeTracker::log(){
	return olotApiLayer::Get().baseLogStream() << olotApiLayer::Get().GetLayerName()
		<< ".Instance[" << pInstance.GetId() << "].EyeGazeTracker: ";
}



// Private Functions
//////////////////////

void olotEyeGazeTracker::pCleanUp(){
	if( pOcsClient ){
		pOcsClient->RemoveUsage();
	}
}
