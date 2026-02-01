#ifndef ______lib_SWISH___status_codes_h
#define ______lib_SWISH___status_codes_h
/**
 * @author Basit Ayantunde (rlamarrr@gmail.com)
 * @brief Templated CURL requests abstraction
 * @version 0.1
 * 
 * @copyright Copyright (c) 2018
 *      __                __         ____                                    __         
 *     /\ \        __    /\ \       /\  _`\                   __            /\ \        
 *     \ \ \      /\_\   \ \ \____  \ \,\L\_\    __  __  __  /\_\     ____  \ \ \___    
 *      \ \ \  __ \/\ \   \ \ '__`\  \/_\__ \   /\ \/\ \/\ \ \/\ \   /',__\  \ \  _ `\  
 *       \ \ \L\ \ \ \ \   \ \ \L\ \   /\ \L\ \ \ \ \_/ \_/ \ \ \ \ /\__, `\  \ \ \ \ \ 
 *        \ \____/  \ \_\   \ \_,__/   \ `\____\ \ \___x___/'  \ \_\\/\____/   \ \_\ \_\
 *         \/___/    \/_/    \/___/     \/_____/  \/__//__/     \/_/ \/___/     \/_/\/_/
 *                                                                                
 *                                                                                
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <string>

#include <curl/curl.h>

namespace swish {

enum Signal : int64_t { kOK = 0, kAbort = 1 };

enum class StatusCode {
  // All fine. Proceed as usual.
  OK = CURLE_OK,

  // The URL you passed to libcurl used a protocol that this libcurl does not
  // support. The support might be a compile-time option that you didn't use, it
  // can be a misspelled protocol string or just a protocol libcurl has no code
  // for
  UnsupportedProtocol = CURLE_UNSUPPORTED_PROTOCOL,

  // Very early initialization code failed. This is likely to be an internal
  // error or problem, or a resource problem where something fundamental
  // couldn't get done at init time.
  InitializationError = CURLE_FAILED_INIT,

  // CURLE_URL_MALFORMAT
  UrlMalformat = CURLE_URL_MALFORMAT,

  // A requested feature, protocol or option was not found built-in in this
  // libcurl due to a build-time decision. This means that a feature or option
  // was not enabled or explicitly disabled when libcurl was built and in order
  // to get it to function you have to get a rebuilt libcurl.
  NotBuiltIn = CURLE_NOT_BUILT_IN,

  // Couldn't resolve proxy. The given proxy host could not be resolved.
  ProxyResolutionError = CURLE_COULDNT_RESOLVE_PROXY,

  // Couldn't resolve host. The given remote host was not resolved.
  HostResolutionError = CURLE_COULDNT_RESOLVE_HOST,

  // Failed to connect() to host or proxy.
  ConnectionError = CURLE_COULDNT_CONNECT,

  // We were denied access to the resource given in the URL. For FTP, this
  // occurs while trying to change to the remote directory.
  RemoteAccessDenied = CURLE_REMOTE_ACCESS_DENIED,

  // A problem was detected in the HTTP2 framing layer. This is somewhat generic
  // and can be one out of several problems, see the error buffer for details.
  HTTP2Error = CURLE_HTTP2,

  // A file transfer was shorter or larger than expected. This happens when the
  // server first reports an expected transfer size, and then delivers data that
  // doesn't match the previously given size.
  PartialFile = CURLE_PARTIAL_FILE,

  // CURLE_QUOTE_ERROR
  QuoteError = CURLE_QUOTE_ERROR,

  // This is returned if CURLOPT_FAILONERROR is set TRUE and the HTTP server
  // returns an error code that is >= 400.
  HTTP_ErrorCode = CURLE_HTTP_RETURNED_ERROR,

  // note as internal error
  WriteCallbackError = CURLE_WRITE_ERROR,

  // Failed starting the upload. For FTP, the server typically denied the STOR
  // command. The error buffer usually contains the server's explanation for
  // this.
  UploadFailed = CURLE_UPLOAD_FAILED,

  // There was a problem reading a local file or an error returned by the read
  // callback.
  ReadCallbackError = CURLE_READ_ERROR,

  // A memory allocation request failed. This is serious badness and things are
  // severely screwed up if this ever occurs.
  OutOfMemory = CURLE_OUT_OF_MEMORY,

  // Operation timeout. The specified time-out period was reached according to
  // the conditions.
  TimedOut = CURLE_OPERATION_TIMEDOUT,

  // The server does not support or accept range requests.
  RangeError = CURLE_RANGE_ERROR,

  // This is an odd error that mainly occurs due to internal confusion.
  HTTP_PostError = CURLE_HTTP_POST_ERROR,

  // A problem occurred somewhere in the SSL/TLS handshake. You really want the
  // error buffer and read the message there as it pinpoints the problem
  // slightly more. Could be certificates (file formats, paths, permissions),
  // passwords, and others.
  SSL_ConnectionError = CURLE_SSL_CONNECT_ERROR,

  // The download could not be resumed because the specified offset was out of
  // the file boundary.
  BadDownloadResumeOffset = CURLE_BAD_DOWNLOAD_RESUME,

  // A file given with FILE:// couldn't be opened. Most likely because the file
  // path doesn't identify an existing file. Did you check file permissions?
  FileReadError = CURLE_FILE_COULDNT_READ_FILE,

  LDAP_CannotBind = CURLE_LDAP_CANNOT_BIND,
  LDAP_SearchFailed = CURLE_LDAP_SEARCH_FAILED,

  // Function not found. A required zlib function was not found.
  ZlibFunctionNotFound = CURLE_FUNCTION_NOT_FOUND,

  // Aborted by callback. A callback returned "abort" to libcurl.
  CallbackAborted = CURLE_ABORTED_BY_CALLBACK,

  // Internal error. A function was called with a bad parameter.
  BadFunctionArgument = CURLE_BAD_FUNCTION_ARGUMENT,

  // Interface error. A specified outgoing interface could not be used. Set
  // which interface to use for outgoing connections' source IP address with
  // CURLOPT_INTERFACE.
  InterfaceFailed = CURLE_INTERFACE_FAILED,

  // Too many redirects. When following redirects, libcurl hit the maximum
  // amount. Set your limit with CURLOPT_MAXREDIRS.
  TooManyRedirects = CURLE_TOO_MANY_REDIRECTS,

  // CURLE_TELNET_OPTION_SYNTAX (49)
  // A telnet option string was Illegally formatted.

  // The remote server's SSL certificate or SSH md5 fingerprint was deemed not
  // OK.
  PeerVerificationFailed = CURLE_PEER_FAILED_VERIFICATION,

  // Nothing was returned from the server, and under the circumstances, getting
  // nothing is considered an error.
  NoServerResponse = CURLE_GOT_NOTHING,

  // Failed sending network data.
  SendError = CURLE_SEND_ERROR,

  // Failure with receiving network data.
  ReceiveError = CURLE_RECV_ERROR,

  // problem with the local client certificate.
  SSL_CertificateProblem = CURLE_SSL_CERTPROBLEM,

  // Couldn't use specified cipher
  SSL_CipherUseError = CURLE_SSL_CIPHER,

  // Unrecognized transfer encoding.
  UnrecognizedContentEncoding = CURLE_BAD_CONTENT_ENCODING,

  // Unrecognized transfer encoding.
  LDAP_InvalidURL = CURLE_LDAP_INVALID_URL,

  // Maximum file size exceeded.
  MaximumFileSizeExceeded = CURLE_FILESIZE_EXCEEDED,

  // Requested FTP SSL level failed.
  SSL_UseFailed = CURLE_USE_SSL_FAILED,

  // When doing a send operation curl had to rewind the data to retransmit, but
  // the rewinding operation failed.
  SendRewindFailed = CURLE_SEND_FAIL_REWIND,

  // Initiating the SSL Engine failed.
  SSL_InitiationFailed = CURLE_SSL_ENGINE_INITFAILED,
  // The remote server denied curl to login
  /**
   *
   *
   *
   *
   *
   *
   *
   *
   */
  ServerLoginDenied = CURLE_LOGIN_DENIED,

  // Out of disk space on the server.
  ServerDiskFull = CURLE_REMOTE_DISK_FULL,

  // The remote server denied curl to login
  CharacterConversionFailed = CURLE_CONV_FAILED,

  // File already exists and will not be overwritten.
  RemoteFileExists = CURLE_REMOTE_FILE_EXISTS,
  // **

  // Character conversion failed.
  ConversionCallbacksRequired = CURLE_CONV_REQD,

  // Problem with reading the SSL CA cert (path? access rights?)
  SSL_CA_CertificateReadError = CURLE_SSL_CACERT_BADFILE,

  // The resource referenced in the URL does not exist.
  RemoteFileNotFound = CURLE_REMOTE_FILE_NOT_FOUND,

  /**
   *
   *
   */

  // Failed to shut down the SSL connection.
  SSL_ShutdownFailed = CURLE_SSL_SHUTDOWN_FAILED,

  // Failed to load CRL file (Added in 7.19.0)
  SSL_CRL_FileReadError = CURLE_SSL_CRL_BADFILE,

  // Issuer check failed (Added in 7.19.0)
  SSL_IssuerError = CURLE_SSL_ISSUER_ERROR,

  /**
   *
   *
   */

  // Mismatch of RTSP CSeq numbers.
  RTSP_CSeq_Mismatch = CURLE_RTSP_CSEQ_ERROR,
  // Mismatch of RTSP Session Identifiers.
  RTSP_SessionMismatch = CURLE_RTSP_SESSION_ERROR,
  /**
   *
   *
   */
  // Chunk callback reported error.
  ChunkCallbackError = CURLE_CHUNK_FAILED,
  /**
   *
   *
   */

  // Stream error in the HTTP/2 framing layer
  HTTP2StreamError = CURLE_HTTP2_STREAM,

  // An API function was called from inside a callback.
  CallbackRecursiveAPI_Call = CURLE_RECURSIVE_API_CALL

};

inline bool IsOK(StatusCode status) { return status == StatusCode::OK; }

// provides interface to curl err buffer
std::string InterpretStatusCode(StatusCode status) {
  CURLcode c_status = static_cast<CURLcode>(status);

  return std::string{curl_easy_strerror(c_status)};
  // memory is freed by curl
  // but copied by std::string
}

// provides interface to curl err buffer
std::string InterpretStatusCode(CURLcode status) {
  return std::string{curl_easy_strerror(status)};
  // memory is freed by curl
  // but copied by std::string
}

};  // namespace swish

#endif
