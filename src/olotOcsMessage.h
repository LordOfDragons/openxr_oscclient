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

#ifndef _OLOTOCSMESSAGE_H_
#define _OLOTOCSMESSAGE_H_

#include <string>
#include <memory>


/**
 * OCS Message.
 */
class olotOcsMessage{
public:
	enum eType{
		etFloat,
		etInteger
	};
	
	struct sParameter{
		eType type;
		float valueFloat;
		uint32_t valueInt;
	};
	
private:
	char pTarget[ 256 ];
	sParameter pParameters[ 16 ];
	int pParameterCount;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** Create OCS Message. */
	olotOcsMessage();
	
	/** Clean up OCS Message. */
	~olotOcsMessage();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** Parse message. */
	bool Parse( const uint8_t *data, size_t length );
	
	/** Target. */
	inline const char *GetTarget() const{ return ( const char* )pTarget; }
	
	/** Parameter count. */
	inline int GetParameterCount() const{ return pParameterCount; }
	
	/** Parameter at index. */
	const sParameter &GetParameterAt( int index ) const;
	/*@}*/
	
	
	
private:
};

#endif
