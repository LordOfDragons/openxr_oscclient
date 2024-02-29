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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exceptions.h"



// Subclass Constructors
//////////////////////////

olotInvalidParam::olotInvalidParam( const std::string &file, int line,
	XrResult result, const std::string &description ) :
	olotException ( "InvalidParam", ! description.empty() ? description
	: std::string( "Invalid Parameter specified" ), file, line, result ){
}

olotNullPointer::olotNullPointer( const std::string &file, int line,
	XrResult result, const std::string &description ) :
	olotException ( "NullPointer", ! description.empty() ? description
	: std::string( "Null Pointer" ), file, line, result ){
}

olotInvalidAction::olotInvalidAction( const std::string &file, int line,
	XrResult result, const std::string &description ) :
	olotException ( "InvalidAction", ! description.empty() ? description
	: std::string( "Invalid Action (internal error)" ), file, line, result ){
}



olotFileNotFound::olotFileNotFound( const std::string &file, int line,
	XrResult result, const std::string &path ) :
	olotException ( "FileNotFound", pCreateText( path ), file, line, result ){
}

std::string olotFileNotFound::pCreateText( const std::string &path ){
	std::string text( "File does not exist" );
	if( ! path.empty() ){
		text += ": ";
		text += path;
	}
	return text;
}



olotFileExists::olotFileExists( const std::string &file, int line,
	XrResult result, const std::string &path ) :
	olotException ( "FileExists", pCreateText( path ), file, line, result ){
}

std::string olotFileExists::pCreateText( const std::string &path ){
	std::string text( "File does exist already" );
	if( ! path.empty() ){
		text += ": ";
		text += path;
	}
	return text;
}



olotOpenFile::olotOpenFile( const std::string &file, int line,
	XrResult result, const std::string &path ) :
	olotException ( "OpenFileFailed", pCreateText( path ), file, line, result ){
}

std::string olotOpenFile::pCreateText( const std::string &path ){
	std::string text( "Open File failed" );
	if( ! path.empty() ){
		text += ": ";
		text += path;
	}
	return text;
}



olotReadFile::olotReadFile( const std::string &file, int line,
	XrResult result, const std::string &path ) :
	olotException ( "ReadFileFailed", pCreateText( path ), file, line, result ){
}

std::string olotReadFile::pCreateText( const std::string &path ){
	std::string text( "Can not read from file" );
	if( ! path.empty() ){
		text += ": ";
		text += path;
	}
	return text;
}



olotWriteFile::olotWriteFile( const std::string &file, int line,
	XrResult result, const std::string &path ) :
	olotException ( "WriteFileFailed", pCreateText( path ), file, line, result ){
}

std::string olotWriteFile::pCreateText( const std::string &path ){
	std::string text( "Can not write to file" );
	if( ! path.empty() ){
		text += ": ";
		text += path;
	}
	return text;
}



olotInvalidFileFormat::olotInvalidFileFormat( const std::string &file, int line,
	XrResult result, const std::string &path ) :
	olotException ( "InvalidFileFormat", pCreateText( path ), file, line, result ){
}

std::string olotInvalidFileFormat::pCreateText( const std::string &path ){
	std::string text( "Invalid File Format" );
	if( ! path.empty() ){
		text += ": ";
		text += path;
	}
	return text;
}



olotDirectoryNotFound::olotDirectoryNotFound( const std::string &file, int line, XrResult result ) :
	olotException ( "DirectoryNotFound", "Directory does not exist", file, line, result ){
}

olotDirectoryRead::olotDirectoryRead( const std::string &file, int line, XrResult result ) :
	olotException ( "DirectoryReadFailed", "Directory read error", file, line, result ){
}

olotAssertion::olotAssertion( const std::string &file, int line,
	XrResult result, const std::string &description ) :
	olotException ( "AssertionException", ! description.empty() ? description
	: std::string( "Assertion exception has occurred" ), file, line, result ){
}
