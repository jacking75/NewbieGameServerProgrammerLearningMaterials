/*
 *  curlite.cpp
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

#include "curlite.hpp"

// anonymous namespace for internal usage
namespace
{
    struct CurlGlobalInitializer
    {
        CurlGlobalInitializer() { curlite::global_init(); }
        ~CurlGlobalInitializer() { curlite::global_cleanup(); }
    };

#ifndef CURLITE_NO_AUTOMATIC_GLOBAL_INITIALIZATION
    CurlGlobalInitializer globalInitializer;
#endif

} // end of anonymous namespace


namespace curlite
{
    template <class HandlerType>
    struct Event
    {
        HandlerType handler;
        void *data;

        Event() : data( nullptr ) { }
    };

    struct Easy::Pimpl
    {
        CURL         *curl;
        CURLcode      err;
        void         *userData;
        bool          throwExceptions;

        Event<ReadHandler>          onRead;
        Event<WriteHandler>         onWrite;
        Event<WriteHandler>         onHeader;
        Event<IoctlHandler>         onIoctl;
        Event<SeekHandler>          onSeek;
        Event<FnMatchHandler>       onFnMatch;
        Event<ProgressHandler>      onProgress;
        Event<XferInfoHandler>      onXferInfo;
        Event<ChunkBeginHandler>    onChunkBegin;
        Event<ChunkEndHandler>      onChunkEnd;
        Event<SockOptHandler>       onSockOpt;
        Event<OpenSocketHandler>    onOpenSocket;
        Event<CloseSocketHandler>   onCloseSocket;
        Event<SslContextHandler>    onSslContext;
        Event<DebugHandler>         onDebug;

        Pimpl();

        // static cURL callbacks
        static size_t read( char *data, size_t size, size_t n, void *userPtr );
        static size_t write( char *data, size_t size, size_t n, void *userPtr );
        static size_t header( char *data, size_t size, size_t n, void *userPtr );
        static int seek( void *userPtr, curl_off_t offset, int origin );
        static int fnMatch( void *userPtr, const char *pattern, const char *string );
        static curlioerr ioctl( CURL *handle, int cmd, void *userPtr );

        static int progress( void *userPtr, double dTotal, double dCurrent, double uTotal, double uCurrent );
        static int xferInfo( void *userPtr, curl_off_t dTotal, curl_off_t dCurrent, curl_off_t uTotal, curl_off_t uCurrent );

        static long chunkBegin( const void *transferInfo, void *userPtr, int remains );
        static long chunkEnd( void *userPtr );

        static curl_socket_t openSocket( void *userPtr, curlsocktype purpose, curl_sockaddr *address );
        static int closeSocket( void *userPtr, curl_socket_t socket );
        static int sockOpt( void *userPtr, curl_socket_t socket, curlsocktype purpose );

        static CURLcode sslContext( CURL *curl, void *sslCtx, void *userPtr );
        static int debug( CURL *, curl_infotype type, char *data, size_t size, void *userPtr );
    };

    /*  Pimpl definitions
     */

    Easy::Pimpl::Pimpl()
    {
        curl = nullptr;
        err = CURLE_OK;
        userData = nullptr;
        throwExceptions = true;
    }

    size_t Easy::Pimpl::read( char *data, size_t size, size_t n, void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onRead;
            if( ev.handler ) {
                return ev.handler( (char*) data, size, n, ev.data );
            }
        }

        return CURL_READFUNC_ABORT;
    }

    size_t Easy::Pimpl::write( char *data, size_t size, size_t n, void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onWrite;
            if( ev.handler ) {
                return ev.handler( (char*) data, size, n, ev.data );
            }
        }

        return 0;
    }

    size_t Easy::Pimpl::header( char *data, size_t size, size_t n, void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onHeader;
            if( ev.handler ) {
                return ev.handler( (char*) data, size, n, ev.data );
            }
        }

        return 0;
    }

    int Easy::Pimpl::fnMatch( void *userPtr, const char *pattern, const char *string )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onFnMatch;
            if( ev.handler ) {
                return ev.handler( ev.data, pattern, string );
            }
        }

        return CURL_FNMATCHFUNC_FAIL;
    }

    int Easy::Pimpl::seek( void *userPtr, curl_off_t offset, int origin )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onSeek;
            if( ev.handler ) {
                return ev.handler( ev.data, offset, origin );
            }
        }

        return 1;
    }

    curlioerr Easy::Pimpl::ioctl( CURL *handle, int cmd, void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onIoctl;
            if( ev.handler ) {
                return ev.handler( handle, cmd, ev.data );
            }
        }

        return CURLIOE_UNKNOWNCMD;
    }

    int Easy::Pimpl::progress( void *userPtr, double dTotal, double dCurrent, double uTotal, double uCurrent )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onProgress;
            if( ev.handler ) {
                return ev.handler( ev.data, dTotal, dCurrent, uTotal, uCurrent );
            }
        }

        return 1;
    }

    int Easy::Pimpl::xferInfo( void *userPtr, curl_off_t dTotal, curl_off_t dCurrent, curl_off_t uTotal, curl_off_t uCurrent )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onXferInfo;
            if( ev.handler ) {
                return ev.handler( ev.data, dTotal, dCurrent, uTotal, uCurrent );
            }
        }

        return 1;
    }

    long Easy::Pimpl::chunkBegin( const void *transferInfo, void *userPtr, int remains )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onChunkBegin;
            if( ev.handler ) {
                return ev.handler( transferInfo, ev.data, remains );
            }
        }

        return CURL_CHUNK_BGN_FUNC_FAIL;
    }

    long Easy::Pimpl::chunkEnd( void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onChunkEnd;
            if( ev.handler ) {
                return ev.handler( ev.data );
            }
        }

        return CURL_CHUNK_END_FUNC_FAIL;
    }

    curl_socket_t Easy::Pimpl::openSocket( void *userPtr, curlsocktype purpose, curl_sockaddr *address )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onOpenSocket;
            if( ev.handler ) {
                return ev.handler( ev.data, purpose, address );
            }
        }

        return CURL_SOCKET_BAD;
    }

    int Easy::Pimpl::closeSocket( void *userPtr, curl_socket_t socket )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onCloseSocket;
            if( ev.handler ) {
                return ev.handler( ev.data, socket );
            }
        }

        return 1;
    }

    int Easy::Pimpl::sockOpt( void *userPtr, curl_socket_t socket, curlsocktype purpose )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onSockOpt;
            if( ev.handler ) {
                return ev.handler( ev.data, socket, purpose );
            }
        }

        return 1;
    }

    CURLcode Easy::Pimpl::sslContext( CURL *curl, void *sslCtx, void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onSslContext;
            if( ev.handler ) {
                return ev.handler( curl, sslCtx, ev.data );
            }
        }

        return CURLE_ABORTED_BY_CALLBACK;
    }

    int Easy::Pimpl::debug( CURL *, curl_infotype type, char *data, size_t size, void *userPtr )
    {
        if( auto impl = reinterpret_cast<Easy::Pimpl*>( userPtr ) )
        {
            auto &ev = impl->onDebug;
            if( ev.handler ) {
                ev.handler( nullptr, type, data, size, ev.data );
            }
        }

        return 0;
    }

    /*  Easy definitions
     */

    Easy::Easy()
        : _impl( new Pimpl() )
    {
        _impl->curl = curl_easy_init();

        if( _impl->curl == nullptr ) {
            throw Exception( "can't init curl_easy interface" );
        }

        // install default options
        set( CURLOPT_USERAGENT, "curlite::Easy" );
    }

    Easy::Easy( Easy &&other )
    {
        *this = std::move( other );
    }

    Easy::~Easy()
    {
        if( auto ptr = release() ) {
            curl_easy_cleanup( ptr );
        }
    }

    Easy &Easy::operator = ( Easy &&other )
    {
        if( this != &other )
        {
            if( auto ptr = release() ) {
                curl_easy_cleanup( ptr );
            }
            
            _impl.swap( other._impl );
        }

        return *this;
    }

    Easy::operator bool() const
    {
        return _impl->err == CURLE_OK;
    }

    bool Easy::operator << (std::istream &stream)
    {
        onRead( [&stream] (char *data, size_t size, size_t n, void *) -> size_t
        {
            stream.read( (char*) data, size * n );
            return size_t( stream.gcount() );
        } );

        return perform();
    }

    bool Easy::operator >> (std::ostream &stream)
    {
        onWrite( [&stream] (char *data, size_t size, size_t n, void *) -> size_t
        {
            stream.write( (char*) data, size * n );
            return stream ? size * n : 0;
        } );

        return perform();
    }

    CURL *Easy::release()
    {
        CURL *curl = nullptr;
        if( _impl ) {
            std::swap( _impl->curl, curl );
        }

        return curl;
    }

    void Easy::reset()
    {
        curl_easy_reset( _impl->curl );

        _impl->err = CURLE_OK;

        onRead();
        onWrite();
        onHeader();
        onProgress();
        onDebug();
    }

    bool Easy::perform()
    {
        return handleError(
            curl_easy_perform( _impl->curl )
        );
    }

    size_t Easy::send( const char *buffer, size_t bufferSize )
    {
        size_t sent = 0;
        auto err = curl_easy_send( _impl->curl, buffer, bufferSize, &sent );
        return handleError( err ) ? sent : 0;
    }

    size_t Easy::recv( char *buffer, size_t bufferSize )
    {
        size_t received = 0;
        auto err = curl_easy_recv( _impl->curl, buffer, bufferSize, &received );
        return handleError( err ) ? received : 0;
    }

    bool Easy::pause( int bitmask )
    {
        return handleError(
            curl_easy_pause( _impl->curl, bitmask )
        );
    }

    CURL* Easy::get() const
    {
        return _impl->curl;
    }

    void Easy::setExceptionMode( bool throwExceptions )
    {
        _impl->throwExceptions = throwExceptions;
    }

    bool Easy::exceptionMode() const
    {
        return _impl->throwExceptions;
    }

    CURLcode Easy::error() const
    {
        return _impl->err;
    }

    std::string Easy::errorString() const
    {
        return curl_easy_strerror( _impl->err );
    }

    bool Easy::handleError( CURLcode code )
    {
        _impl->err = code;
        
        if( _impl->err != CURLE_OK && _impl->throwExceptions ) {
            throw Exception( curl_easy_strerror(_impl->err) );
        }

        return _impl->err == CURLE_OK;
    }

    void Easy::setUserData( void *data )
    {
        _impl->userData = data;
    }

    void *Easy::userData() const
    {
        return _impl->userData;
    }

    std::string Easy::escape( std::string const &url )
    {
        std::string escaped;

        char *str = curl_easy_escape( _impl->curl, url.data(), url.size() );
        if( str ) {
            escaped = str;
            curl_free( str );
        }

        return escaped; 
    }

    std::string Easy::unescape( std::string const &url )
    {
        std::string unescaped;

        int outLength = 0; // who the hell decided to use int in curl_easy_unescape?
        char *str = curl_easy_unescape( _impl->curl, url.data(), url.size(), &outLength );
        if( str ) {
            unescaped.assign( str, str + outLength );
            curl_free( str );
        }

        return unescaped;
    }

    void Easy::onRead_( SimplifiedDataHandler f )
    {
        auto wrapper = [f]( char *data, size_t size, size_t n, void * ) -> size_t {
            return f( data, size * n ) ? size * n : CURL_READFUNC_ABORT;
        };

        onRead( f ? wrapper : ReadHandler() );
    }

    void Easy::onWrite_( SimplifiedDataHandler f )
    {
        auto wrapper = [f]( char *data, size_t size, size_t n, void * ) -> size_t {
            return f( data, size * n ) ? size * n : 0;
        };

        onWrite( f ? wrapper : WriteHandler() );
    }

    void Easy::onHeader_( SimplifiedDataHandler f )
    {
        auto wrapper = [f]( char *data, size_t size, size_t n, void * ) -> size_t {
            return f( data, size * n ) ? size * n : 0;
        };

        onHeader( f ? wrapper : WriteHandler() );
    }

    void Easy::onProgress_( SimplifiedProgressHandler f )
    {
        auto wrapper = [f]( void *, curl_off_t dTotal, curl_off_t dCurrent, curl_off_t uTotal, curl_off_t uCurrent ) -> int {
            return !f( dTotal, dCurrent, uTotal, uCurrent );
        };

        onXferInfo( f ? wrapper : XferInfoHandler() );
    }

    void Easy::onRead( ReadHandler f, void *data )
    {
        _impl->onRead.handler = f;
        _impl->onRead.data = data;

        set( CURLOPT_READFUNCTION, f ? &Pimpl::read : nullptr );
        set( CURLOPT_READDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onWrite( WriteHandler f, void *data )
    {
        _impl->onWrite.handler = f;
        _impl->onWrite.data = data;

        set( CURLOPT_WRITEFUNCTION, f ? &Pimpl::write : nullptr );
        set( CURLOPT_WRITEDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onHeader( WriteHandler f, void *data)
    {
        _impl->onHeader.handler = f;
        _impl->onHeader.data = data;

        set( CURLOPT_HEADERFUNCTION, f ? &Pimpl::header : nullptr );
        set( CURLOPT_HEADERDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onProgress( ProgressHandler f, void *data )
    {
        _impl->onProgress.handler = f;
        _impl->onProgress.data = data;

        set( CURLOPT_PROGRESSFUNCTION, f ? &Pimpl::progress : nullptr );
        set( CURLOPT_PROGRESSDATA, f ? (void*) this->_impl.get() : nullptr );
        set( CURLOPT_NOPROGRESS, f ? false : true );
    }

    void Easy::onDebug( DebugHandler f, void *data)
    {
        _impl->onDebug.handler = f;
        _impl->onDebug.data = data;

        set( CURLOPT_DEBUGFUNCTION, f ? &Pimpl::debug : nullptr );
        set( CURLOPT_DEBUGDATA, f ? (void*) this->_impl.get() : nullptr );
        set( CURLOPT_VERBOSE, f ? true : false );
    }

    void Easy::onIoctl( IoctlHandler f, void *data)
    {
        _impl->onIoctl.handler = f;
        _impl->onIoctl.data = data;

        set( CURLOPT_IOCTLFUNCTION, f ? &Pimpl::ioctl : nullptr );
        set( CURLOPT_IOCTLDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onSeek( SeekHandler f, void *data)
    {
        _impl->onSeek.handler = f;
        _impl->onSeek.data = data;

        set( CURLOPT_SEEKFUNCTION, f ? &Pimpl::seek : nullptr );
        set( CURLOPT_SEEKDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onFnMatch( FnMatchHandler f, void *data)
    {
        _impl->onFnMatch.handler = f;
        _impl->onFnMatch.data = data;

        set( CURLOPT_FNMATCH_FUNCTION, f ? &Pimpl::fnMatch : nullptr );
        set( CURLOPT_FNMATCH_DATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onXferInfo( XferInfoHandler f, void *data)
    {
        _impl->onXferInfo.handler = f;
        _impl->onXferInfo.data = data;

        set( CURLOPT_XFERINFOFUNCTION, f ? &Pimpl::xferInfo : nullptr );
        set( CURLOPT_XFERINFODATA, f ? (void*) this->_impl.get() : nullptr );
        set( CURLOPT_NOPROGRESS, f ? false : true );
    }

    void Easy::onOpenSocket( OpenSocketHandler f, void *data)
    {
        _impl->onOpenSocket.handler = f;
        _impl->onOpenSocket.data = data;

        set( CURLOPT_OPENSOCKETFUNCTION, f ? &Pimpl::openSocket : nullptr );
        set( CURLOPT_OPENSOCKETDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onCloseSocket( CloseSocketHandler f, void *data)
    {
        _impl->onCloseSocket.handler = f;
        _impl->onCloseSocket.data = data;

        set( CURLOPT_CLOSESOCKETFUNCTION, f ? &Pimpl::closeSocket : nullptr );
        set( CURLOPT_CLOSESOCKETDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onSockOpt( SockOptHandler f, void *data)
    {
        _impl->onSockOpt.handler = f;
        _impl->onSockOpt.data = data;

        set( CURLOPT_SOCKOPTFUNCTION, f ? &Pimpl::sockOpt : nullptr );
        set( CURLOPT_SOCKOPTDATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onChunkBegin( ChunkBeginHandler f, void *data)
    {
        _impl->onChunkBegin.handler = f;
        _impl->onChunkBegin.data = data;

        set( CURLOPT_CHUNK_BGN_FUNCTION, f ? &Pimpl::chunkBegin : nullptr );
        set( CURLOPT_CHUNK_DATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onChunkEnd( ChunkEndHandler f, void *data)
    {
        _impl->onChunkEnd.handler = f;
        _impl->onChunkEnd.data = data;

        set( CURLOPT_CHUNK_END_FUNCTION, f ? &Pimpl::chunkEnd : nullptr );
        set( CURLOPT_CHUNK_DATA, f ? (void*) this->_impl.get() : nullptr );
    }

    void Easy::onSslContext( SslContextHandler f, void *data)
    {
        _impl->onSslContext.handler = f;
        _impl->onSslContext.data = data;

        set( CURLOPT_SSL_CTX_FUNCTION, f ? &Pimpl::sslContext : nullptr );
        set( CURLOPT_SSL_CTX_DATA, f ? (void*) this->_impl.get() : nullptr );
    }

    std::ostream &operator << ( std::ostream &stream, Easy &curlite )
    {
        curlite >> stream;
        return stream;
    }

    std::istream &operator >> ( std::istream &stream, Easy &curlite )
    {
        curlite << stream;
        return stream;
    }

    /* Definition of curlite::List
     */

    List::List( curl_slist *list )
        : _list( list )
    {
    }

    List::List( std::vector<std::string> const &values )
        : _list( nullptr )
    {
        append( values );
    }

    List::~List()
    {
        if( auto ptr = release() ) {
            curl_slist_free_all( ptr );
        }
    }

    List::List( List &&other )
        : _list( nullptr )
    {
        *this = std::move( other );
    }

    List &List::operator = ( List &&other )
    {
        if( this != &other )
        {
            if( auto ptr = release() ) {
                curl_slist_free_all( ptr );
            }
            
            std::swap( _list, other._list );
        }

        return *this;
    }

    curl_slist *List::get() const
    {
        return _list;
    }

    curl_slist *List::release()
    {
        curl_slist *list = nullptr;
        std::swap( list, _list );
        return list;
    }

    List &List::append( char const *s )
    {
        _list = curl_slist_append( _list, s );
        return *this;
    }

    List &List::append( std::vector<std::string> const &values )
    {
        for( auto it = values.begin(); it != values.end(); ++it ) {
            append( it->c_str() );
        }

        return *this;
    }

    List &List::operator << ( char const *s )
    {
        return append( s );
    }

    /* Definition of curlite::Form
     */

    Form::Form() : _first( nullptr ), _last( nullptr )
    {
    }

    Form::Form( std::vector<curl_forms> const &forms )
        : _first( nullptr ), _last( nullptr )
    {
        add( forms );
    }

    Form::Form( Form &&other )
        : _first( nullptr ), _last( nullptr )
    {
        std::swap( _first, other._first );
        std::swap( _last, other._last );
    }

    Form::~Form()
    {
        if( auto ptr = release().first ) {
            curl_formfree( ptr );
        }
    }

    Form &Form::operator = ( Form &&other )
    {
        if( this != &other )
        {
            if( auto ptr = release().first ) {
                curl_formfree( ptr );
            }
            
            std::swap( _first, other._first );
            std::swap( _last, other._last );
        }

        return *this;
    }

    curl_httppost *Form::get() const
    {
        return _first;
    }

    std::pair<curl_httppost*, curl_httppost*> Form::release()
    {
        auto r = std::make_pair<curl_httppost*, curl_httppost*>( nullptr, nullptr );

        std::swap( r.first, _first );
        std::swap( r.second, _last );
        
        return r;
    }

    bool Form::add( std::vector<curl_forms> const &forms )
    {
        auto err = curl_formadd( &_first, &_last, CURLFORM_ARRAY, forms.data(), CURLFORM_END );
        return err == CURL_FORMADD_OK;
    }

    /* Other functions
     */

    bool global_init( long flags )
    {
        return curl_global_init( flags ) == 0;
    }

    void global_cleanup()
    {
        curl_global_cleanup();
    }

    std::string version()
    {
        return std::string( curl_version() );
    }

    curl_version_info_data *versionInfo( CURLversion type )
    {
        return curl_version_info( type );
    }

    Easy download( std::string const &url, std::ostream &ostr, bool followRedirect, bool throwExceptions )
    {
        Easy c;
        c.setExceptionMode( throwExceptions );
        c.set( CURLOPT_URL, url );
        c.set( CURLOPT_FOLLOWLOCATION, followRedirect );

        c >> ostr;

        return std::move( c );
    }

    Easy upload( std::istream &istr,
                 std::string const &url,
                 std::string const &username,
                 std::string const &password,
                 curl_off_t size,
                 bool throwExceptions )
    {
        Easy c;
        c.setExceptionMode( throwExceptions );
        c.set( CURLOPT_URL, url );
        c.set( CURLOPT_USERNAME, username );
        c.set( CURLOPT_PASSWORD, password );
        c.set( CURLOPT_INFILESIZE_LARGE, size );
        c.set( CURLOPT_UPLOAD, true );

        List headers;
        if( size == -1 ) {
            headers << "Transfer-Encoding: chunked";
            headers << "Expect:";

            // if it's not http(s) upload then the option will be ignored
            c.set( CURLOPT_HTTPHEADER, headers.get() );
        }

        istr >> c;

        // reset the option to avoid access violation (if a client reuses the Easy object)
        c.set( CURLOPT_HTTPHEADER, nullptr );

        return std::move( c );
    }

} // end of namespace <curlite>
