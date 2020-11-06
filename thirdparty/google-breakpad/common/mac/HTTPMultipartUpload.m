// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "HTTPMultipartUpload.h"

#import "GTMDefines.h"
#import "encoding_util.h"

@interface HTTPMultipartUpload (PrivateMethods)
- (NSString*)multipartBoundary;
// Each of the following methods will append the starting multipart boundary,
// but not the ending one.
- (NSData*)formDataForKey:(NSString*)key value:(NSString*)value;
- (NSData*)formDataForFileContents:(NSData*)contents name:(NSString*)name;
- (NSData*)formDataForFile:(NSString*)file name:(NSString*)name;
@end

@implementation HTTPMultipartUpload
//=============================================================================
#pragma mark -
#pragma mark || Private ||
//=============================================================================
- (NSString*)multipartBoundary {
  // The boundary has 27 '-' characters followed by 16 hex digits
  return [NSString
      stringWithFormat:@"---------------------------%08X%08X", rand(), rand()];
}

//=============================================================================
- (NSData*)formDataForKey:(NSString*)key value:(NSString*)value {
  NSMutableData* data = [NSMutableData data];
  [self appendBoundaryData:data];

  NSString* escaped = PercentEncodeNSString(key);
  NSString* fmt = @"Content-Disposition: form-data; name=\"%@\"\r\n\r\n%@\r\n";
  NSString *form = [NSString stringWithFormat:fmt, escaped, value];

  [data appendData:[form dataUsingEncoding:NSUTF8StringEncoding]];
  return data;
}

//=============================================================================
- (void)appendBoundaryData:(NSMutableData*)data {
  NSString* fmt = @"--%@\r\n";
  NSString* pre = [NSString stringWithFormat:fmt, boundary_];

  [data appendData:[pre dataUsingEncoding:NSUTF8StringEncoding]];
}

//=============================================================================
#pragma mark -
#pragma mark || Public ||
//=============================================================================
- (id)initWithURL:(NSURL*)url {
  if ((self = [super initWithURL:url])) {
    boundary_ = [[self multipartBoundary] retain];
    files_ = [[NSMutableDictionary alloc] init];
  }

  return self;
}

//=============================================================================
- (void)dealloc {
  [parameters_ release];
  [files_ release];
  [boundary_ release];

  [super dealloc];
}

//=============================================================================
- (void)setParameters:(NSDictionary*)parameters {
  if (parameters != parameters_) {
    [parameters_ release];
    parameters_ = [parameters copy];
  }
}

//=============================================================================
- (NSDictionary*)parameters {
  return parameters_;
}

//=============================================================================
- (void)addFileAtPath:(NSString*)path name:(NSString*)name {
  [files_ setObject:path forKey:name];
}

//=============================================================================
- (void)addFileContents:(NSData*)data name:(NSString*)name {
  [files_ setObject:data forKey:name];
}

//=============================================================================
- (NSDictionary*)files {
  return files_;
}

//=============================================================================
- (NSString*)HTTPMethod {
  return @"POST";
}

//=============================================================================
- (NSString*)contentType {
  return [NSString
      stringWithFormat:@"multipart/form-data; boundary=%@", boundary_];
}

//=============================================================================
- (NSData*)bodyData {
  NSMutableData* postBody = [NSMutableData data];

  // Add any parameters to the message
  NSArray* parameterKeys = [parameters_ allKeys];
  NSString* key;

  NSInteger count = [parameterKeys count];
  for (NSInteger i = 0; i < count; ++i) {
    key = [parameterKeys objectAtIndex:i];
    [postBody appendData:[self formDataForKey:key
                                        value:[parameters_ objectForKey:key]]];
  }

  // Add any files to the message
  NSArray* fileNames = [files_ allKeys];
  for (NSString* name in fileNames) {
    // First append boundary
    [self appendBoundaryData:postBody];
    // Then the formdata
    id fileOrData = [files_ objectForKey:name];
    [HTTPRequest appendFileToBodyData:postBody
                             withName:name
                       withFileOrData:fileOrData];
  }

  NSString* epilogue = [NSString stringWithFormat:@"\r\n--%@--\r\n", boundary_];
  [postBody appendData:[epilogue dataUsingEncoding:NSUTF8StringEncoding]];

  return postBody;
}

@end
