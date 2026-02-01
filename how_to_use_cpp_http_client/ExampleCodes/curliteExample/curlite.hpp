/*
 *  curlite.hpp
 *
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013-2014 Ivan Grynko
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef curlite_hpp_120129ad_36b2_4224_b790_d1658ce03bb4
#define curlite_hpp_120129ad_36b2_4224_b790_d1658ce03bb4

#include <curl/curl.h>
#include <stdexcept>
#include <functional>
#include <memory>
#include <iostream>
#include <vector>
#include <string>

// cURL version check
#if LIBCURL_VERSION_MAJOR < 7
//|| LIBCURL_VERSION_MINOR < 32
    #error "This version of curlite is incompatible with your cURL version" 
#endif

namespace curlite
{
    template <class FunctionPtr>
    struct Handler { typedef std::function<typename std::remove_pointer<FunctionPtr>::type> type; };

    typedef Handler<curl_progress_callback>::type    ProgressHandler;
    typedef Handler<curl_xferinfo_callback>::type    XferInfoHandler;
    typedef Handler<curl_write_callback>::type       WriteHandler;
    typedef Handler<curl_chunk_bgn_callback>::type   ChunkBeginHandler;
    typedef Handler<curl_chunk_end_callback>::type   ChunkEndHandler;
    typedef Handler<curl_fnmatch_callback>::type     FnMatchHandler;
    typedef Handler<curl_seek_callback>::type        SeekHandler;
    typedef Handler<curl_read_callback>::type        ReadHandler;
    typedef Handler<curl_sockopt_callback>::type     SockOptHandler;
    typedef Handler<curl_opensocket_callback>::type  OpenSocketHandler;
    typedef Handler<curl_closesocket_callback>::type CloseSocketHandler;
    typedef Handler<curl_ioctl_callback>::type       IoctlHandler;
    typedef Handler<curl_debug_callback>::type       DebugHandler;
    typedef Handler<curl_conv_callback>::type        ConvHandler;
    typedef Handler<curl_ssl_ctx_callback>::type     SslContextHandler;
    typedef Handler<curl_formget_callback>::type     FormGetHandler;

    /* Base class of all curlite exceptions
     */

    struct Exception : std::runtime_error
    {
        Exception( char const *message )
         : std::runtime_error( message )
        { }
    };

    /* The class implements easy interface of cURL
     * 
     * Example:
     *     try {
     *         curlite::Easy easy;
     *         easy.set( CURLOPT_URL, "http://duckduckgo.com" );
     *         easy.set( CURLOPT_FOLLOWLOCATION, true );
     *     
     *         std::cout << easy;
     *     } catch( std::exception &e ) {
     *         std::err << e.what() << std::endl;
     *     }
     */

    class Easy
    {
        struct Pimpl;
        std::unique_ptr<Pimpl> _impl;

        Easy( Easy const &other );
        void operator = ( Easy const &other );

        bool handleError( CURLcode code );

    public:
        // Simplified handlers for most frequent cases
        typedef std::function<bool (char *, size_t)> SimplifiedDataHandler;
        typedef std::function<bool (curl_off_t, curl_off_t, curl_off_t, curl_off_t)> SimplifiedProgressHandler;

        Easy();
        Easy( Easy &&other );
        virtual ~Easy();

        Easy &operator = ( Easy &&other );

        /* Returns true if there was no error during last operation
         * Equivalent of (error() == CURLE_OK)
         */

        operator bool() const;

        /* Release the ownership of the managed CURL object if any.
         *
         * Returns pointer to the managed CURL object or nullptr.
         */
        
        CURL *release();

        /* Returns pointer to the managed CURL object
         */

        CURL* get() const;

        /* Set current exception mode.
         * Pass true to throw exceptions on error, false otherwise.
         */

        void setExceptionMode( bool throwExceptions );

        /* Returns true if exceptions are "on"
         */

        bool exceptionMode() const;

        /* Returns last cURL error code
         */

        CURLcode error() const;

        /* Returns a string describing last cURL error
         */

        std::string errorString() const;

        /* Returns previously set pointer to user data
         */

        void *userData() const;

        /* Set pointer to arbitrary user data, associated with the instance of Easy
         */

        void setUserData( void *data );

        /* Set options for the cURL session
         * See curl_easy_setopt() for details.
         */

        template <class ValueType>
        bool set( CURLoption opt, ValueType value );

        bool set( CURLoption key, int value );
        bool set( CURLoption key, bool value );
        bool set( CURLoption key, std::string const &value );

        /* Request internal information from cURL session
         * See curl_easy_getinfo() for details.
         */

        template <class ValueType>
        ValueType getInfo( CURLINFO key, ValueType const &defaultValue = ValueType() );

        /* Reset all options of cURL session to their defaults.
         */

        void reset();

        /* Pause/unpause a connection
         * See curl_easy_pause() for details.
         */

        bool pause( int bitmask );

        /* Perform a blocking file transfer.
         */

        bool perform();

        /* Perform a blocking file upload from a stream
         */

        bool operator << ( std::istream &stream );

        /* Perform a blocking file download to a stream
         */

        bool operator >> ( std::ostream &stream );

        /* Returns url-encoded version of input string
         */

        std::string escape( std::string const &url );

        /* Returns url-decoded version of input string
         */

        std::string unescape( std::string const &url );

        /* Send arbitrary data over the established connection.
         * Returns the number of bytes actually sent.
         */

        size_t send( const char *buffer, size_t bufferSize );

        /* Receive raw data from the established connection.
         * Returns the number of bytes actually received.
         */

        size_t recv( char *buffer, size_t bufferSize );

        // set simplified handlers
        void onRead_( SimplifiedDataHandler f );
        void onWrite_( SimplifiedDataHandler f );
        void onHeader_( SimplifiedDataHandler f );

        void onProgress_( SimplifiedProgressHandler f );

        // set usual "curl" handlers
        void onRead( ReadHandler f = ReadHandler(), void *data = nullptr );
        void onWrite( WriteHandler f = WriteHandler(), void *data = nullptr );
        void onHeader( WriteHandler f = WriteHandler(), void *data = nullptr );

        void onIoctl( IoctlHandler f = IoctlHandler(), void *data = nullptr );
        void onSeek( SeekHandler f = SeekHandler(), void *data = nullptr );
        void onFnMatch( FnMatchHandler f = FnMatchHandler(), void *data = nullptr );

        void onProgress( ProgressHandler f = ProgressHandler(), void *data = nullptr );
        void onXferInfo( XferInfoHandler f = XferInfoHandler(), void *data = nullptr );

        void onCloseSocket( CloseSocketHandler f = CloseSocketHandler(), void *data = nullptr );
        void onOpenSocket( OpenSocketHandler f = OpenSocketHandler(), void *data = nullptr );
        void onSockOpt( SockOptHandler f = SockOptHandler(), void *data = nullptr );

        void onChunkBegin( ChunkBeginHandler f = ChunkBeginHandler(), void *data = nullptr );
        void onChunkEnd( ChunkEndHandler f = ChunkEndHandler(), void *data = nullptr );

        void onSslContext( SslContextHandler f = SslContextHandler(), void *data = nullptr );
        void onDebug( DebugHandler f = DebugHandler(), void *data = nullptr );
    };


    // -- set() stuff
    static const int kCurlOptTypeInterval = CURLOPTTYPE_OBJECTPOINT - CURLOPTTYPE_LONG;

    struct OptionInvalidCode     { enum { value = -1 }; };
    struct OptionNullPtrCode     { enum { value = -2 }; };
    struct OptionLongCode        { enum { value = CURLOPTTYPE_LONG }; };
    struct OptionOffsetCode      { enum { value = CURLOPTTYPE_OFF_T }; };
    struct OptionObjectPtrCode   { enum { value = CURLOPTTYPE_OBJECTPOINT }; };
    struct OptionFunctionPtrCode { enum { value = CURLOPTTYPE_FUNCTIONPOINT }; };

    template <class Type> struct OptionTypeCode                  : OptionInvalidCode { };
    template <> struct OptionTypeCode<long>                      : OptionLongCode { };
    template <> struct OptionTypeCode<void*>                     : OptionObjectPtrCode { };
    template <> struct OptionTypeCode<char*>                     : OptionObjectPtrCode { };
    template <> struct OptionTypeCode<const char*>               : OptionObjectPtrCode { };
    template <> struct OptionTypeCode<curl_slist*>               : OptionObjectPtrCode { };
    template <> struct OptionTypeCode<curl_httppost*>            : OptionObjectPtrCode { };
    template <> struct OptionTypeCode<FILE*>                     : OptionObjectPtrCode { };
    template <> struct OptionTypeCode<curl_progress_callback>    : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_xferinfo_callback>    : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_write_callback>       : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_chunk_bgn_callback>   : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_chunk_end_callback>   : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_fnmatch_callback>     : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_seek_callback>        : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_sockopt_callback>     : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_opensocket_callback>  : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_closesocket_callback> : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_ioctl_callback>       : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_debug_callback>       : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_conv_callback>        : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_ssl_ctx_callback>     : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<curl_formget_callback>     : OptionFunctionPtrCode { };
    template <> struct OptionTypeCode<std::nullptr_t>            : OptionNullPtrCode { };
    // disable specialization if curl_off_t and long is the same
    template <> struct OptionTypeCode<std::conditional<std::is_same<long, curl_off_t>::value, void, curl_off_t>::type> : OptionOffsetCode { };


    template <class ValueType>
    bool Easy::set( CURLoption key, ValueType value )
    {
        static_assert( int(OptionTypeCode<ValueType>::value) != int(OptionInvalidCode::value), "the type is not supported by curl_easy_setopt" );

        auto err = CURLE_OK;

        // realtime argument check
        auto keyTypeCode = key / kCurlOptTypeInterval * kCurlOptTypeInterval;
        bool isValueAllowedNullPtr = std::is_same<ValueType, std::nullptr_t>::value &&
                                     keyTypeCode != CURLOPTTYPE_LONG;

        if( OptionTypeCode<ValueType>::value != keyTypeCode && !isValueAllowedNullPtr ) {
            err = CURLE_BAD_FUNCTION_ARGUMENT;
        } else {
            err = curl_easy_setopt( get(), key, value );
        }

        return handleError( err );
    }

    inline bool Easy::set( CURLoption key, int value )
    {
        return set( key, static_cast<long>( value ) );
    }

    inline bool Easy::set( CURLoption key, bool value )
    {
        return set( key, static_cast<long>( value ) );
    }

    inline bool Easy::set( CURLoption key, std::string const &value )
    {
        return set( key, value.c_str() );
    }

    // -- getInfo() stuff
    template <class Type> struct InfoTypeCode             { enum { value = -1              }; };
    template <> struct InfoTypeCode<char*>                { enum { value = CURLINFO_STRING }; };
    template <> struct InfoTypeCode<long>                 { enum { value = CURLINFO_LONG   }; };
    template <> struct InfoTypeCode<double>               { enum { value = CURLINFO_DOUBLE }; };
    template <> struct InfoTypeCode<curl_slist*>          { enum { value = CURLINFO_SLIST  }; };
    template <> struct InfoTypeCode<curl_certinfo*>       { enum { value = CURLINFO_SLIST  }; };
    template <> struct InfoTypeCode<curl_tlssessioninfo*> { enum { value = CURLINFO_SLIST  }; };

    template <class ValueType>
    ValueType Easy::getInfo( CURLINFO key, ValueType const &defaultValue )
    {
        ValueType value;
        auto err = CURLE_OK;

        static_assert( InfoTypeCode<ValueType>::value != -1, "the type is not supported by curl_easy_getinfo" );

        if( InfoTypeCode<ValueType>::value != (key & CURLINFO_TYPEMASK) ) {
            err = CURLE_BAD_FUNCTION_ARGUMENT;
        } else {
            err = curl_easy_getinfo( get(), key, &value );
        }

        return handleError( err ) ? value : defaultValue;
    }

    template <>
    inline std::string Easy::getInfo( CURLINFO key, std::string const &defaultValue )
    {
        const char* value = getInfo<char*>( key, nullptr );
        return value ? value : defaultValue;
    }

    std::ostream &operator << ( std::ostream &stream, Easy &curlite );
    std::istream &operator >> ( std::istream &stream, Easy &curlite );

    /* Wrapper arround curl list (curl_slist)
     * 
     * Example:
     *     List list ({{
     *         "pragma:"
     *     }});
     */

    class List
    {
        curl_slist *_list;

        List( List const &other );
        void operator = ( List const &other );
    public:
        List( curl_slist *list = nullptr );
        List( std::vector<std::string> const &values );
        List( List &&other );

        ~List();

        List &operator = ( List &&other );

        /* Returns pointer to the managed curl_slist object or nullptr.
         */

        curl_slist *get() const;

        /* Release the ownership of the managed curl_httppost object if any.
         *
         * Returns pointer to the managed curl_slist object or nullptr.
         */
        curl_slist *release();

        /* Add new item to the list. 
         */

        List &append( char const *s );
        List &append( std::vector<std::string> const &values );

        /* Just an alias for append().
         */
        List &operator << ( char const *s );
    };

    /* Wrapper arround curl forms (curl_httppost)
     * 
     * Example:
     *     Form form ({
     *         { CURLFORM_COPYNAME,     "name"      },
     *         { CURLFORM_COPYCONTENTS, "contents"  }
     *     });
     */

    class Form
    {
        curl_httppost *_first;
        curl_httppost *_last;

        Form( Form const &other );
        void operator = ( Form const &other );
    public:

        Form();
        Form( std::vector<curl_forms> const &forms );
        Form( Form &&other );

        ~Form();

        Form &operator = ( Form &&other );

        /* Returns pointer to the first item of managed curl_httppost object or nullptr.
         */

        curl_httppost *get() const;

        /* Release the ownership of the managed curl_httppost object if any.
         *
         * Returns std::pair with pointers to the managed curl_httppost object:
         *     pair.first   pointer to first curl_httppost item
         *     pair.second  pointer to last curl_httppost item
         */

        std::pair<curl_httppost*, curl_httppost*> release();

        /* Add a section to the form. Supported all CURLFORM_* options except CURLFORM_ARRAY.
         *
         * Note: It's up to you to manage resources passed to the function. Be careful.
         */

        bool add( std::vector<curl_forms> const &forms );
    };

    /* Synonym for curl_global_init(). Returns true on success.
     */

    bool global_init( long flags = CURL_GLOBAL_ALL );

    /* Synonym for curl_global_cleanup().
     */

    void global_cleanup();

    /* Synonym for curl_version(). Returns cURL version string.
     *
     * Note: using versionInfo() should be preferred, if possible.
     */

    std::string version();

    /* Synonym for curl_version_info(). Returns pointer to static curl_version_info_data structure.
     *
     * See cURL function curl_version_info() for more details.
     */

    curl_version_info_data *versionInfo( CURLversion type = CURLVERSION_NOW );

    /* Download resource at a particular URL
     *
     *     url                resource to download
     *     ostr               stream to write the resource data to
     *     followRedirect     internally sets CURLOPT_FOLLOWLOCATION option to 1, if true
     */

    Easy download( std::string const &url, std::ostream &ostr, bool followRedirect = true, bool throwExceptions = true );

    /* Upload resource to a particular URL
     *
     *     istr               stream to read the resource data from
     *     url                url to upload input stream to
     *     username           username to use in authentication
     *     password           password to use in authentication
     *     size               total size of the upload (if known)
     */

    Easy upload( std::istream &istr,
                 std::string const &url,
                 std::string const &username = "",
                 std::string const &password = "",
                 curl_off_t size = -1,
                 bool throwExceptions = true );



} // end of namespace

#endif
