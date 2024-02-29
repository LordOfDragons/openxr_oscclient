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
			//const int result = pOcsClient->GetLastEyeData( data );
			
			/*
			if( result == v::WORK && data.no_user ){
				sampleTime = ( XrTime )data.timestamp;
				
				if( hasOpennessRight || hasOpennessLeft ){
					
					// reading the data is a bit convoluted.
					// 
					// the blinking has to be linked to the eye openess value from the verbose data.
					// an openness value of 1 maps to a blinking value of 0. hence openness of 1
					// produces a 0 weight for the blinking face shape. an openness value of 0
					// maps to a blinking value of 1. hence openness of 0 produces a 1 weight for
					// blinking face shape.
					pWeights[ XR_EYE_EXPRESSION_RIGHT_BLINK_HTC ] = clamp( 1.0f - data.verbose_data.right.eye_openness );
					pWeights[ XR_EYE_EXPRESSION_LEFT_BLINK_HTC ] = clamp( 1.0f - data.verbose_data.left.eye_openness );
					
					// the wide value has to be linked to the eye wide value from the expression data.
					// a value of 0 produces 0 weight for wide face shape. a value of 1 produces 1.
					// if value is larger than 0 blinking has to be 0 according to specification.
					// the wide value obtained from the vive is quite unreliable. for normal looking
					// ahead this can easily go up to 0.9 for no obvious reason. calibration seems
					// tricky for this value.
					pWeights[ XR_EYE_EXPRESSION_RIGHT_WIDE_HTC ] = clamp( data.expression_data.right.eye_wide );
					if( pWeights[ XR_EYE_EXPRESSION_RIGHT_WIDE_HTC ] > 0.01f ){
						pWeights[ XR_EYE_EXPRESSION_RIGHT_BLINK_HTC ] = 0.0f;
					}

					pWeights[ XR_EYE_EXPRESSION_LEFT_WIDE_HTC ] = clamp( data.expression_data.left.eye_wide );
					if( pWeights[ XR_EYE_EXPRESSION_LEFT_WIDE_HTC ] > 0.01f ){
						pWeights[ XR_EYE_EXPRESSION_LEFT_BLINK_HTC ] = 0.0f;
					}

					// the squeeze value has to be linked to the eye squeeze value from the expression data.
					// a value of 0 produces 0 weight for squeeze face shape. a value of 1 produces 1.
					// if value is larger than 0 blinking has to be 1 according to specification.
					// the squeeze value seems to go at most up to 0.5 . could be problem with calibration.
					// to get a reasonable result the squeeze value is muliplied by 2 to cover the entire range
					const float squeezeFactor = 1.0f; //2.0f

					pWeights[ XR_EYE_EXPRESSION_RIGHT_SQUEEZE_HTC ] =
						clamp( data.expression_data.right.eye_squeeze * squeezeFactor );
					if( pWeights[ XR_EYE_EXPRESSION_RIGHT_SQUEEZE_HTC ] > 0.01f ){
						pWeights[ XR_EYE_EXPRESSION_RIGHT_BLINK_HTC ] = 1.0f;
					}

					pWeights[ XR_EYE_EXPRESSION_LEFT_SQUEEZE_HTC ] =
						clamp( data.expression_data.left.eye_squeeze * squeezeFactor );
					if( pWeights[ XR_EYE_EXPRESSION_LEFT_SQUEEZE_HTC ] > 0.01f ){
						pWeights[ XR_EYE_EXPRESSION_LEFT_BLINK_HTC ] = 1.0f;
					}

					// there is no explicit value mapping to the eye down, up, left and right values
					
					pActive = true;
					
				}else{
					{
					const std::lock_guard<std::mutex> guard( olotApiLayer::Get().mutexLog );
					log() << "eye openness not valid for both right and left eye" << std::endl;
					}
					
					pActive = false;
				}
				
			}else{
				pActive = false;
			}
			*/
			pActive = false;
			
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
