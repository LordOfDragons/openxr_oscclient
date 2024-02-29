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

#include <string>
#include <stdlib.h>
#include <string.h>

#include "olotOcsMessage.h"
#include "exceptions/exceptions.h"


// class olotOcsMessage
/////////////////////////

olotOcsMessage::olotOcsMessage() :
pParameterCount( 0 )
{
	memset( pTarget, 0, sizeof( pTarget ) );
	memset( pParameters, 0, sizeof( pParameters ) );
}

olotOcsMessage::~olotOcsMessage(){
}



// Management
///////////////

bool olotOcsMessage::Parse( const uint8_t *data, size_t length ){
	size_t i = 0;
	pParameterCount = 0;
	
	// target
	int next = 0;
	while( i < length ){
		const uint8_t c = data[ i++ ];
		if( ! c ){
			i += ( 4 - i % 4) % 4;
			break;
		}
		if( next == 255 ){
			return false;
		}
		pTarget[ next++ ] = ( char )c;
	}
	pTarget[ next ] = 0;
	
	// types
	if( i == length || data[ i ] != ',' ){
		return false;
	}
	i++;
	
	int parameter = 0;
	while( i < length ){
		const uint8_t c = data[ i++ ];
		if( ! c ){
			i += ( 4 - i % 4) % 4;
			break;
		}
		if( parameter == 16 ){
			return false;
		}
		
		switch( c ){
		case 'f':
			pParameters[ parameter++ ].type = etFloat;
			break;
			
		case 'i':
			pParameters[ parameter++ ].type = etInteger;
			break;
			
		default:
			return false;
		}
	}
	
	// parameters
	for( pParameterCount=0; pParameterCount<parameter; pParameterCount++ ){
		sParameter &p = pParameters[ pParameterCount ];
		
		switch( p.type ){
		case etFloat:
			p.valueInt = ( uint32_t )data[ i ] << 24
				| ( ( uint32_t )data[ i + 1 ] << 16 )
				| ( ( uint32_t )data[ i + 2 ] << 8 )
				| ( ( uint32_t )data[ i + 3 ] );
			i += 4;
			memcpy( &p.valueFloat, &p.valueInt, 4 );
			break;
			
		case etInteger:
			p.valueInt = ( uint32_t )data[ i ] << 24
				| ( ( uint32_t )data[ i + 1 ] << 16 )
				| ( ( uint32_t )data[ i + 2 ] << 8 )
				| ( ( uint32_t )data[ i + 3 ] );
			i += 4;
			break;
		}
	}
	
	return true;
}

const olotOcsMessage::sParameter &olotOcsMessage::GetParameterAt( int index ) const{
	OLOTASSERT_TRUE( index >= 0, XR_ERROR_RUNTIME_FAILURE )
	OLOTASSERT_TRUE( index < pParameterCount, XR_ERROR_RUNTIME_FAILURE )
	return pParameters[ index ];
}
