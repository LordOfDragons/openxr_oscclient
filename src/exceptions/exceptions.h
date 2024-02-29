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

#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include "olotException.h"
#include "../openxr/openxr.h"


/** Invalid parameter exception. */
class olotInvalidParam : public olotException
{
public:
	/** Create a new exception object. */
	olotInvalidParam( const std::string &file, int line, XrResult result, const std::string &description = "" );
};

/** Null Pointer Exception. */
class olotNullPointer : public olotException
{
public:
	/** Create a new exception object. */
	olotNullPointer( const std::string &file, int line, XrResult result, const std::string &description = "" );
};

/** Invalid Action Exception. */
class olotInvalidAction : public olotException
{
public:
	/** Create a new exception object. */
	olotInvalidAction( const std::string &file, int line, XrResult result, const std::string &description = "" );
};

/** File not found Exception. */
class olotFileNotFound : public olotException
{
public:
	/** Create a new exception object. */
	olotFileNotFound( const std::string &file, int line, XrResult result, const std::string &path = "" );
	
private:
	static std::string pCreateText( const std::string &path );
};

/** File does already exist Exception. */
class olotFileExists : public olotException
{
public:
	/** Create a new exception object. */
	olotFileExists( const std::string &file, int line, XrResult result, const std::string &path = "" );
	
private:
	static std::string pCreateText( const std::string &path );
};

/** File can not be opened Exception. */
class olotOpenFile : public olotException
{
public:
	/** Create a new exception object. */
	olotOpenFile( const std::string &file, int line, XrResult result, const std::string &path = "" );
	
private:
	static std::string pCreateText( const std::string &path );
};

/** File can not be read Exception. */
class olotReadFile : public olotException
{
public:
	/** Create a new exception object. */
	olotReadFile( const std::string &file, int line, XrResult result, const std::string &path = "" );
	
private:
	static std::string pCreateText( const std::string &path );
};

/** File can not be written Exception. */
class olotWriteFile : public olotException
{
public:
	/** Create a new exception object. */
	olotWriteFile( const std::string &file, int line, XrResult result, const std::string &path = "" );
	
private:
	static std::string pCreateText( const std::string &path );
};

/** Invalid File Format Exception. */
class olotInvalidFileFormat : public olotException
{
public:
	/** Create a new exception object. */
	olotInvalidFileFormat( const std::string &file, int line, XrResult result, const std::string &path = "" );
	
private:
	static std::string pCreateText( const std::string &path );
};

/** Directory not found Exception. */
class olotDirectoryNotFound : public olotException
{
public:
	/** Create a new exception object. */
	olotDirectoryNotFound( const std::string &file, int line, XrResult result );
};

/** Can not read directory Exception. */
class olotDirectoryRead : public olotException
{
public:
	/** Create a new exception object. */
	olotDirectoryRead( const std::string &file, int line, XrResult result );
};

/** Assertion Exception. */
class olotAssertion : public olotException
{
public:
	/** Create a new exception object. */
	olotAssertion( const std::string &file, int line, XrResult result, const std::string &description = "" );
};


/** Throw an exception of the given type. */
#define OLOTTHROW( cls, result ) throw cls( __FILE__, __LINE__, result )

/**
 * Throw an exception of the given type with additional information.
 * 
 * The type specified has to be an exception class subclassing olotException. The file and line
 * information are obtained from the location the macro is expanded. In additiona the
 * information field of the exception is assigned the constant string 'info'.
 */
#define OLOTTHROW_INFO( cls, result, info ) throw cls( __FILE__, __LINE__, result, info )

/** Throw olotNullPointer exception if pointer is nullptr. */
#define OLOTASSERT_NOTNULL( pointer, result )\
	if( ! ( pointer ) ){\
		OLOTTHROW_INFO( olotNullPointer, result, "assertNotNull(" #pointer ")" );\
	}

/** Throw olotInvalidParam exception if pointer is not nullptr. */
#define OLOTASSERT_NULL( pointer, result )\
	if( pointer ){\
		OLOTTHROW_INFO( olotInvalidParam, result, "assertNull(" #pointer ")" );\
	}

/** Throw olotInvalidParam exception if condition is false. */
#define OLOTASSERT_TRUE( condition, result )\
	if( ! ( condition ) ) {\
		OLOTTHROW_INFO( olotInvalidParam, result, "assertTrue(" #condition ")" );\
	}

/** Throw olotInvalidParam exception if condition is true. */
#define OLOTASSERT_FALSE( condition, result )\
	if( condition ){\
		OLOTTHROW_INFO( olotInvalidParam, result, "assertFalse(" #condition ")" );\
	}

/** Throw olotInvalidAction exception if not succeeded. */
#define OLOTASSERT_SUCCESS( result )\
	if( XR_FAILED( result ) ){\
		OLOTTHROW_INFO( olotInvalidAction, result, "assertSuccess(" #result ")" );\
	}

#endif
