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

#include <chrono>
#include <string.h>
#include <math.h>

#include "olotFacialTracker.h"
#include "olotInstance.h"
#include "olotInstance.h"
#include "olotApiLayer.h"
#include "olotOcsClient.h"
#include "exceptions/exceptions.h"



// class olotFacialTracker
////////////////////////////

olotFacialTracker::olotFacialTracker( olotInstance &instance,
	const XrFacialTrackerCreateInfoHTC &createInfo ) :
pInstance( instance ),
pWeights( nullptr ),
pWeightCount( 0 ),
pActive( false ),
pOcsClient( nullptr ),
pDestroyed( false )
{
	try{
		switch( createInfo.facialTrackingType ){
		case XR_FACIAL_TRACKING_TYPE_EYE_DEFAULT_HTC:
			pCreateEyeTracker();
			break;
			
		case XR_FACIAL_TRACKING_TYPE_LIP_DEFAULT_HTC:
			pCreateLipTracker();
			break;
			
		default:
			OLOTASSERT_SUCCESS( XR_ERROR_VALIDATION_FAILURE )
		}
		
	}catch( const olotException & ){
		pCleanUp();
		throw;
	}
}

olotFacialTracker::~olotFacialTracker(){
	pCleanUp();
}



// Management
///////////////

static inline float clamp( float value ){
	return std::max( std::min( value, 1.0f ), 0.0f );
}

static inline float linearStep( float value, float from, float to ){
	return clamp( ( value - from ) / ( to - from ) );
}

static inline float linearStep( float value, float from, float to, float mapFrom, float mapTo ){
	return linearStep( value, from, to ) * ( mapTo - mapFrom ) + mapFrom;
}

static inline float vec2Length( float x, float y ){
	return sqrtf( x * x + y * y );
}

static const float invSqrt2 = 1.0f / sqrtf( 2.0f );

XrResult olotFacialTracker::DestroyFacialTracker(){
	OLOTASSERT_FALSE( pDestroyed, XR_ERROR_HANDLE_INVALID )
	
	pDestroyed = true;
	return pInstance.DestroyFacialTracker( this );
}

XrResult olotFacialTracker::GetFacialExpressionsHTC( XrFacialExpressionsHTC *facialExpressions ){
	OLOTASSERT_FALSE( pDestroyed, XR_ERROR_HANDLE_INVALID )
	OLOTASSERT_TRUE( facialExpressions->expressionCount == pWeightCount, XR_ERROR_VALIDATION_FAILURE )
	
	const XrTime sampleTime = ( XrTime )std::clock();
	
	try{
		pOcsClient->GetExpressionValues( pOcsValues, olotOcsClient::ExpressionCount );
		
		if( pType == etEye ){
			pActive = true;
			
		}else{
			pWeights[ XR_LIP_EXPRESSION_JAW_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeJawRight ];
			pWeights[ XR_LIP_EXPRESSION_JAW_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeJawLeft ];
			pWeights[ XR_LIP_EXPRESSION_JAW_FORWARD_HTC ] = pOcsValues[ olotOcsClient::eeJawForward ];
			pWeights[ XR_LIP_EXPRESSION_JAW_OPEN_HTC ] = pOcsValues[ olotOcsClient::eeJawOpen ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_POUT_HTC ] = pOcsValues[ olotOcsClient::eeMouthPucker ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_SMILE_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeMouthSmileRight ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_SMILE_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeMouthSmileLeft ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_SAD_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeMouthFrownRight ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_SAD_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeMouthFrownLeft ];
			pWeights[ XR_LIP_EXPRESSION_CHEEK_PUFF_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeCheekPuffRight ];
			pWeights[ XR_LIP_EXPRESSION_CHEEK_PUFF_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeCheekPuffLeft ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_UPPER_UPRIGHT_HTC ] = pOcsValues[ olotOcsClient::eeMouthUpperUpRight ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_UPPER_UPLEFT_HTC ] = pOcsValues[ olotOcsClient::eeMouthUpperUpLeft ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_DOWNRIGHT_HTC ] = pOcsValues[ olotOcsClient::eeMouthLowerDownRight ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_DOWNLEFT_HTC ] = pOcsValues[ olotOcsClient::eeMouthLowerDownLeft ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_UPPER_INSIDE_HTC ] = pOcsValues[ olotOcsClient::eeMouthRollUpper ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_INSIDE_HTC ] = pOcsValues[ olotOcsClient::eeMouthRollLower ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_OVERLAY_HTC ] = pOcsValues[ olotOcsClient::eeMouthShrugLower ];
			pWeights[ XR_LIP_EXPRESSION_TONGUE_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeTongueLeft ];
			pWeights[ XR_LIP_EXPRESSION_TONGUE_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeTongueRight ];
			pWeights[ XR_LIP_EXPRESSION_TONGUE_UP_HTC ] = pOcsValues[ olotOcsClient::eeTongueUp ];
			pWeights[ XR_LIP_EXPRESSION_TONGUE_DOWN_HTC ] = pOcsValues[ olotOcsClient::eeTongueDown ];
			pWeights[ XR_LIP_EXPRESSION_TONGUE_ROLL_HTC ] = pOcsValues[ olotOcsClient::eeTongueRoll ];
			
			pWeights[ XR_LIP_EXPRESSION_MOUTH_APE_SHAPE_HTC ] = pOcsValues[ olotOcsClient::eeMouthClose ];
			
			pWeights[ XR_LIP_EXPRESSION_MOUTH_UPPER_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeMouthRight ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_UPPER_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeMouthLeft ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_RIGHT_HTC ] = pOcsValues[ olotOcsClient::eeMouthRight ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_LEFT_HTC ] = pOcsValues[ olotOcsClient::eeMouthLeft ];
			
			pWeights[ XR_LIP_EXPRESSION_MOUTH_UPPER_OVERTURN_HTC ] = pOcsValues[ olotOcsClient::eeMouthFunnel ];
			pWeights[ XR_LIP_EXPRESSION_MOUTH_LOWER_OVERTURN_HTC ] = pOcsValues[ olotOcsClient::eeMouthFunnel ];
			
			pWeights[ XR_LIP_EXPRESSION_CHEEK_SUCK_HTC ] = std::max(
				pOcsValues[ olotOcsClient::eeCheekSuckRight ], pOcsValues[ olotOcsClient::eeCheekSuckLeft ] );
			
			const float tongueOut = pOcsValues[ olotOcsClient::eeTongueOut ];
			const float tongueUp = pOcsValues[ olotOcsClient::eeTongueUp ];
			const float tongueDown = pOcsValues[ olotOcsClient::eeTongueDown ];
			const float tongueRight = pOcsValues[ olotOcsClient::eeTongueRight ];
			const float tongueLeft = pOcsValues[ olotOcsClient::eeTongueLeft ];
			
			pWeights[ XR_LIP_EXPRESSION_TONGUE_LONGSTEP1_HTC ] = linearStep( tongueOut, 0.0f, 0.5f, 0.0f, 1.0f );
			pWeights[ XR_LIP_EXPRESSION_TONGUE_LONGSTEP2_HTC ] = linearStep( tongueOut, 0.5f, 1.0f, 0.0f, 1.0f );
			
			pWeights[ XR_LIP_EXPRESSION_TONGUE_UPRIGHT_MORPH_HTC ] = vec2Length( tongueUp, tongueRight ) * invSqrt2 * tongueOut;
			pWeights[ XR_LIP_EXPRESSION_TONGUE_UPLEFT_MORPH_HTC ] = vec2Length( tongueUp, tongueLeft ) * invSqrt2 * tongueOut;
			pWeights[ XR_LIP_EXPRESSION_TONGUE_DOWNRIGHT_MORPH_HTC ] = vec2Length( tongueDown, tongueRight ) * invSqrt2 * tongueOut;
			pWeights[ XR_LIP_EXPRESSION_TONGUE_DOWNLEFT_MORPH_HTC ] = vec2Length( tongueDown, tongueLeft ) * invSqrt2 * tongueOut;
			
			/*
			not mapped ocs values:
			- olotOcsClient::eeNoseSneerLeft
			- olotOcsClient::eeNoseSneerRight
			- olotOcsClient::eeMouthShrugUpper
			- olotOcsClient::eeMouthDimpleLeft
			- olotOcsClient::eeMouthDimpleRight
			- olotOcsClient::eeMouthPressLeft
			- olotOcsClient::eeMouthPressRight
			- olotOcsClient::eeMouthStretchLeft
			- olotOcsClient::eeMouthStretchRight
			- olotOcsClient::eeTongueBendDown
			- olotOcsClient::eeTongueCurlUp
			- olotOcsClient::eeTongueSquish
			- olotOcsClient::eeTongueFlat
			- olotOcsClient::eeTongueTwistLeft
			- olotOcsClient::eeTongueTwistRight
			*/
			pActive = true;
		}
		
	}catch( const olotException & ){
		pActive = false;
	}
	
	facialExpressions->sampleTime = sampleTime;
	
	memcpy( facialExpressions->expressionWeightings, pWeights, sizeof( float ) * pWeightCount );
	
	facialExpressions->isActive = pActive ? XR_TRUE : XR_FALSE;
	
	return XR_SUCCESS;
}

std::ostream &olotFacialTracker::log(){
	return olotApiLayer::Get().baseLogStream() << olotApiLayer::Get().GetLayerName()
		<< ".Instance[" << pInstance.GetId() << "].FacialTracker: ";
}



// Private Functions
//////////////////////

void olotFacialTracker::pCleanUp(){
	if( pOcsClient ){
		pOcsClient->RemoveUsage();
	}
	
	if( pWeights ){
		delete [] pWeights;
	}
}

void olotFacialTracker::pCreateEyeTracker(){
	pType = etEye;
	
	pWeights = new float[ 14 ];
	pWeightCount = 14;
	memset( pWeights, 0, sizeof( float ) * pWeightCount );
	
	pOcsClient = olotApiLayer::Get().AcquireOcsClient();
}

void olotFacialTracker::pCreateLipTracker(){
	pType = etLip;
	
	pWeights = new float[ 37 ];
	pWeightCount = 37;
	memset( pWeights, 0, sizeof( float ) * pWeightCount );
	
	pOcsClient = olotApiLayer::Get().AcquireOcsClient();
}
