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

XrResult olotEyeGazeTracker::GetActionStatePose( XrActionStatePose &state ){
	//const int result = pOcsClient->GetLastEyeData( data );
	
	/*
	if( result == v::WORK && data.no_user ){
		const vae::SingleEyeData &ced = data.verbose_data.combined.eye_data;
		const uint64_t cedv = ced.eye_data_validata_bit_mask;
		
		if( vae::DecodeBitMask( cedv, vae::SINGLE_EYE_DATA_GAZE_ORIGIN_VALIDITY )
		&& vae::DecodeBitMask( cedv, vae::SINGLE_EYE_DATA_GAZE_DIRECTION_VALIDITY ) ){
			// store position. the specification does not state this explicitely but from the
			// way data is reported the coordinte system used is relative to the eye camera
			// not the head. the coordinate system used is:
			// x: positive to the right
			// y: positive upwards
			// z: positive backwards
			pPose.position.x = ced.gaze_origin_mm.x / 1e4f;
			pPose.position.y = ced.gaze_origin_mm.y / 1e4f;
			pPose.position.z = ced.gaze_origin_mm.z / 1e4f;
			
			// calculate orientation matching direction. we first convert into Drag[en]gine
			// coordinate system so we can use the queaternion implementation used there.
			// these coordinates are still relative to the eye camera. coordinate system is:
			// x: positive to the right
			// y: positive upwards
			// z: positive forward
			//
			// hence z has to be flipped
			XrVector3f dir;
			dir.x =  ced.gaze_direction_normalized.x;
			dir.y =  ced.gaze_direction_normalized.y;
			dir.z = -ced.gaze_direction_normalized.z;

			// calculate the horizontal and vertical rotation angles. for looking straight
			// ahead this will be a rotation by 180 around y axis
			const float rotY = -atan2f( -dir.x, dir.z );
			const float rotX = -atan2f(  dir.y, sqrtf( dir.x * dir.x + dir.z * dir.z ) );
			
			// create quaternion from euler angles and store it
			olotQuaternion orientation( olotQuaternion::CreateFromEuler( rotX, rotY, 0.0f ) );

			pPose.orientation.x = orientation.x;
			pPose.orientation.y = orientation.y;
			pPose.orientation.z = orientation.z;
			pPose.orientation.w = orientation.w;
			
			pActive = true;
			
		}else{
			pActive = false;
		}
		
	}else{
		pActive = false;
	}
	*/
	pActive = false;
	
	state.type = XR_TYPE_ACTION_STATE_POSE;
	state.next = nullptr;
	state.isActive = pActive ? XR_TRUE : XR_FALSE;
	
	return XR_SUCCESS;
}

XrResult olotEyeGazeTracker::LocateSpace( const olotSpace &space,
XrSpace baseSpace, XrTime time, XrSpaceLocation *location ){
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
