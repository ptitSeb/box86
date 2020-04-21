#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "bridge.h"
#include "librarian.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "debug.h"
#include "box86context.h"
#include "emu/x86emu_private.h"
#include "callback.h"

#include "khash.h"

const char* curlName = "libcurl.so.4";
#define LIBNAME curl

typedef struct curldata_s {
    x86emu_t*   writefnc;
    void*       writedata;
    x86emu_t*   readfnc;
    void*       readdata;
    x86emu_t*   iofnc;
    void*       iodata;
    x86emu_t*   seekfnc;
    void*       seekdata;
    x86emu_t*   headerfnc;
    void*       headerdata;
} curldata_t;

KHASH_MAP_INIT_INT(curldata, curldata_t)


typedef uint32_t (*uFpup_t)(void*, uint32_t, void*);
typedef void (*vFp_t)(void*);

typedef struct curl_my_s {
    // functions
    vFp_t           curl_easy_cleanup;
    uFpup_t         curl_easy_setopt;
    // data
    kh_curldata_t   *curldata;
} curl_my_t;

static void clean_cdata(curl_my_t *my, void* handle)
{
    khint_t k;
    k = kh_get(curldata, my->curldata, (uintptr_t)handle);
    if(k!=kh_end(my->curldata)) {
        curldata_t* cdata = &kh_value(my->curldata, k);
        #define GO(A) if(cdata->A) FreeCallback(cdata->A)
        GO(writefnc);
        GO(readfnc);
        GO(iofnc);
        GO(seekfnc);
        GO(headerfnc);
        #undef GO
        kh_del(curldata, my->curldata, k);
    }
}

void* getCurlMy(library_t* lib)
{
    curl_my_t* my = (curl_my_t*)calloc(1, sizeof(curl_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(curl_easy_cleanup, vFp_t)
    GO(curl_easy_setopt, uFpup_t)
    #undef GO
    my->curldata = kh_init(curldata);
    return my;
}

void freeCurlMy(void* lib)
{
    curl_my_t *my = (curl_my_t *)lib;
    kh_destroy(curldata, my->curldata);
}

#define LONG          0
#define OBJECTPOINT   10000
#define STRINGPOINT   10000
#define FUNCTIONPOINT 20000
#define OFF_T         30000
#define CINIT(name,type,number) CURLOPT_ ## name = type + number

typedef enum {
  CINIT(WRITEDATA, OBJECTPOINT, 1),
  CINIT(URL, STRINGPOINT, 2),
  CINIT(PORT, LONG, 3),
  CINIT(PROXY, STRINGPOINT, 4),
  CINIT(USERPWD, STRINGPOINT, 5),
  CINIT(PROXYUSERPWD, STRINGPOINT, 6),
  CINIT(RANGE, STRINGPOINT, 7),
  CINIT(READDATA, OBJECTPOINT, 9),
  CINIT(ERRORBUFFER, OBJECTPOINT, 10),
  CINIT(WRITEFUNCTION, FUNCTIONPOINT, 11),
  CINIT(READFUNCTION, FUNCTIONPOINT, 12),
  CINIT(TIMEOUT, LONG, 13),
  CINIT(INFILESIZE, LONG, 14),
  CINIT(POSTFIELDS, OBJECTPOINT, 15),
  CINIT(REFERER, STRINGPOINT, 16),
  CINIT(FTPPORT, STRINGPOINT, 17),
  CINIT(USERAGENT, STRINGPOINT, 18),
  CINIT(LOW_SPEED_LIMIT, LONG, 19),
  CINIT(LOW_SPEED_TIME, LONG, 20),
  CINIT(RESUME_FROM, LONG, 21),
  CINIT(COOKIE, STRINGPOINT, 22),
  CINIT(HTTPHEADER, OBJECTPOINT, 23),
  CINIT(HTTPPOST, OBJECTPOINT, 24),
  CINIT(SSLCERT, STRINGPOINT, 25),
  CINIT(KEYPASSWD, STRINGPOINT, 26),
  CINIT(CRLF, LONG, 27),
  CINIT(QUOTE, OBJECTPOINT, 28),
  CINIT(HEADERDATA, OBJECTPOINT, 29),
  CINIT(COOKIEFILE, STRINGPOINT, 31),
  CINIT(SSLVERSION, LONG, 32),
  CINIT(TIMECONDITION, LONG, 33),
  CINIT(TIMEVALUE, LONG, 34),
  CINIT(CUSTOMREQUEST, STRINGPOINT, 36),
  CINIT(STDERR, OBJECTPOINT, 37),
  CINIT(POSTQUOTE, OBJECTPOINT, 39),
  CINIT(OBSOLETE40, OBJECTPOINT, 40),
  CINIT(VERBOSE, LONG, 41),
  CINIT(HEADER, LONG, 42),
  CINIT(NOPROGRESS, LONG, 43),
  CINIT(NOBODY, LONG, 44),
  CINIT(FAILONERROR, LONG, 45),
  CINIT(UPLOAD, LONG, 46),
  CINIT(POST, LONG, 47),
  CINIT(DIRLISTONLY, LONG, 48),
  CINIT(APPEND, LONG, 50),
  CINIT(NETRC, LONG, 51),
  CINIT(FOLLOWLOCATION, LONG, 52),
  CINIT(TRANSFERTEXT, LONG, 53),
  CINIT(PUT, LONG, 54),
  CINIT(PROGRESSFUNCTION, FUNCTIONPOINT, 56),
  CINIT(PROGRESSDATA, OBJECTPOINT, 57),
#define CURLOPT_XFERINFODATA CURLOPT_PROGRESSDATA
  CINIT(AUTOREFERER, LONG, 58),
  CINIT(PROXYPORT, LONG, 59),
  CINIT(POSTFIELDSIZE, LONG, 60),
  CINIT(HTTPPROXYTUNNEL, LONG, 61),
  CINIT(INTERFACE, STRINGPOINT, 62),
  CINIT(KRBLEVEL, STRINGPOINT, 63),
  CINIT(SSL_VERIFYPEER, LONG, 64),
  CINIT(CAINFO, STRINGPOINT, 65),
  CINIT(MAXREDIRS, LONG, 68),
  CINIT(FILETIME, LONG, 69),
  CINIT(TELNETOPTIONS, OBJECTPOINT, 70),
  CINIT(MAXCONNECTS, LONG, 71),
  CINIT(OBSOLETE72, LONG, 72),
  CINIT(FRESH_CONNECT, LONG, 74),
  CINIT(FORBID_REUSE, LONG, 75),
  CINIT(RANDOM_FILE, STRINGPOINT, 76),
  CINIT(EGDSOCKET, STRINGPOINT, 77),
  CINIT(CONNECTTIMEOUT, LONG, 78),
  CINIT(HEADERFUNCTION, FUNCTIONPOINT, 79),
  CINIT(HTTPGET, LONG, 80),
  CINIT(SSL_VERIFYHOST, LONG, 81),
  CINIT(COOKIEJAR, STRINGPOINT, 82),
  CINIT(SSL_CIPHER_LIST, STRINGPOINT, 83),
  CINIT(HTTP_VERSION, LONG, 84),
  CINIT(FTP_USE_EPSV, LONG, 85),
  CINIT(SSLCERTTYPE, STRINGPOINT, 86),
  CINIT(SSLKEY, STRINGPOINT, 87),
  CINIT(SSLKEYTYPE, STRINGPOINT, 88),
  CINIT(SSLENGINE, STRINGPOINT, 89),
  CINIT(SSLENGINE_DEFAULT, LONG, 90),
  CINIT(DNS_USE_GLOBAL_CACHE, LONG, 91),
  CINIT(DNS_CACHE_TIMEOUT, LONG, 92),
  CINIT(PREQUOTE, OBJECTPOINT, 93),
  CINIT(DEBUGFUNCTION, FUNCTIONPOINT, 94),
  CINIT(DEBUGDATA, OBJECTPOINT, 95),
  CINIT(COOKIESESSION, LONG, 96),
  CINIT(CAPATH, STRINGPOINT, 97),
  CINIT(BUFFERSIZE, LONG, 98),
  CINIT(NOSIGNAL, LONG, 99),
  CINIT(SHARE, OBJECTPOINT, 100),
  CINIT(PROXYTYPE, LONG, 101),
  CINIT(ACCEPT_ENCODING, STRINGPOINT, 102),
  CINIT(PRIVATE, OBJECTPOINT, 103),
  CINIT(HTTP200ALIASES, OBJECTPOINT, 104),
  CINIT(UNRESTRICTED_AUTH, LONG, 105),
  CINIT(FTP_USE_EPRT, LONG, 106),
  CINIT(HTTPAUTH, LONG, 107),
  CINIT(SSL_CTX_FUNCTION, FUNCTIONPOINT, 108),
  CINIT(SSL_CTX_DATA, OBJECTPOINT, 109),
  CINIT(FTP_CREATE_MISSING_DIRS, LONG, 110),
  CINIT(PROXYAUTH, LONG, 111),
  CINIT(FTP_RESPONSE_TIMEOUT, LONG, 112),
#define CURLOPT_SERVER_RESPONSE_TIMEOUT CURLOPT_FTP_RESPONSE_TIMEOUT
  CINIT(IPRESOLVE, LONG, 113),
  CINIT(MAXFILESIZE, LONG, 114),
  CINIT(INFILESIZE_LARGE, OFF_T, 115),
  CINIT(RESUME_FROM_LARGE, OFF_T, 116),
  CINIT(MAXFILESIZE_LARGE, OFF_T, 117),
  CINIT(NETRC_FILE, STRINGPOINT, 118),
  CINIT(USE_SSL, LONG, 119),
  CINIT(POSTFIELDSIZE_LARGE, OFF_T, 120),
  CINIT(TCP_NODELAY, LONG, 121),
  CINIT(FTPSSLAUTH, LONG, 129),
  CINIT(IOCTLFUNCTION, FUNCTIONPOINT, 130),
  CINIT(IOCTLDATA, OBJECTPOINT, 131),
  CINIT(FTP_ACCOUNT, STRINGPOINT, 134),
  CINIT(COOKIELIST, STRINGPOINT, 135),
  CINIT(IGNORE_CONTENT_LENGTH, LONG, 136),
  CINIT(FTP_SKIP_PASV_IP, LONG, 137),
  CINIT(FTP_FILEMETHOD, LONG, 138),
  CINIT(LOCALPORT, LONG, 139),
  CINIT(LOCALPORTRANGE, LONG, 140),
  CINIT(CONNECT_ONLY, LONG, 141),
  CINIT(CONV_FROM_NETWORK_FUNCTION, FUNCTIONPOINT, 142),
  CINIT(CONV_TO_NETWORK_FUNCTION, FUNCTIONPOINT, 143),
  CINIT(CONV_FROM_UTF8_FUNCTION, FUNCTIONPOINT, 144),
  CINIT(MAX_SEND_SPEED_LARGE, OFF_T, 145),
  CINIT(MAX_RECV_SPEED_LARGE, OFF_T, 146),
  CINIT(FTP_ALTERNATIVE_TO_USER, STRINGPOINT, 147),
  CINIT(SOCKOPTFUNCTION, FUNCTIONPOINT, 148),
  CINIT(SOCKOPTDATA, OBJECTPOINT, 149),
  CINIT(SSL_SESSIONID_CACHE, LONG, 150),
  CINIT(SSH_AUTH_TYPES, LONG, 151),
  CINIT(SSH_PUBLIC_KEYFILE, STRINGPOINT, 152),
  CINIT(SSH_PRIVATE_KEYFILE, STRINGPOINT, 153),
  CINIT(FTP_SSL_CCC, LONG, 154),
  CINIT(TIMEOUT_MS, LONG, 155),
  CINIT(CONNECTTIMEOUT_MS, LONG, 156),
  CINIT(HTTP_TRANSFER_DECODING, LONG, 157),
  CINIT(HTTP_CONTENT_DECODING, LONG, 158),
  CINIT(NEW_FILE_PERMS, LONG, 159),
  CINIT(NEW_DIRECTORY_PERMS, LONG, 160),
  CINIT(POSTREDIR, LONG, 161),
  CINIT(SSH_HOST_PUBLIC_KEY_MD5, STRINGPOINT, 162),
  CINIT(OPENSOCKETFUNCTION, FUNCTIONPOINT, 163),
  CINIT(OPENSOCKETDATA, OBJECTPOINT, 164),
  CINIT(COPYPOSTFIELDS, OBJECTPOINT, 165),
  CINIT(PROXY_TRANSFER_MODE, LONG, 166),
  CINIT(SEEKFUNCTION, FUNCTIONPOINT, 167),
  CINIT(SEEKDATA, OBJECTPOINT, 168),
  CINIT(CRLFILE, STRINGPOINT, 169),
  CINIT(ISSUERCERT, STRINGPOINT, 170),
  CINIT(ADDRESS_SCOPE, LONG, 171),
  CINIT(CERTINFO, LONG, 172),
  CINIT(USERNAME, STRINGPOINT, 173),
  CINIT(PASSWORD, STRINGPOINT, 174),
  CINIT(PROXYUSERNAME, STRINGPOINT, 175),
  CINIT(PROXYPASSWORD, STRINGPOINT, 176),
  CINIT(NOPROXY, STRINGPOINT, 177),
  CINIT(TFTP_BLKSIZE, LONG, 178),
  CINIT(SOCKS5_GSSAPI_SERVICE, STRINGPOINT, 179),
  CINIT(SOCKS5_GSSAPI_NEC, LONG, 180),
  CINIT(PROTOCOLS, LONG, 181),
  CINIT(REDIR_PROTOCOLS, LONG, 182),
  CINIT(SSH_KNOWNHOSTS, STRINGPOINT, 183),
  CINIT(SSH_KEYFUNCTION, FUNCTIONPOINT, 184),
  CINIT(SSH_KEYDATA, OBJECTPOINT, 185),
  CINIT(MAIL_FROM, STRINGPOINT, 186),
  CINIT(MAIL_RCPT, OBJECTPOINT, 187),
  CINIT(FTP_USE_PRET, LONG, 188),
  CINIT(RTSP_REQUEST, LONG, 189),
  CINIT(RTSP_SESSION_ID, STRINGPOINT, 190),
  CINIT(RTSP_STREAM_URI, STRINGPOINT, 191),
  CINIT(RTSP_TRANSPORT, STRINGPOINT, 192),
  CINIT(RTSP_CLIENT_CSEQ, LONG, 193),
  CINIT(RTSP_SERVER_CSEQ, LONG, 194),
  CINIT(INTERLEAVEDATA, OBJECTPOINT, 195),
  CINIT(INTERLEAVEFUNCTION, FUNCTIONPOINT, 196),
  CINIT(WILDCARDMATCH, LONG, 197),
  CINIT(CHUNK_BGN_FUNCTION, FUNCTIONPOINT, 198),
  CINIT(CHUNK_END_FUNCTION, FUNCTIONPOINT, 199),
  CINIT(FNMATCH_FUNCTION, FUNCTIONPOINT, 200),
  CINIT(CHUNK_DATA, OBJECTPOINT, 201),
  CINIT(FNMATCH_DATA, OBJECTPOINT, 202),
  CINIT(RESOLVE, OBJECTPOINT, 203),
  CINIT(TLSAUTH_USERNAME, STRINGPOINT, 204),
  CINIT(TLSAUTH_PASSWORD, STRINGPOINT, 205),
  CINIT(TLSAUTH_TYPE, STRINGPOINT, 206),
  CINIT(TRANSFER_ENCODING, LONG, 207),
  CINIT(CLOSESOCKETFUNCTION, FUNCTIONPOINT, 208),
  CINIT(CLOSESOCKETDATA, OBJECTPOINT, 209),
  CINIT(GSSAPI_DELEGATION, LONG, 210),
  CINIT(DNS_SERVERS, STRINGPOINT, 211),
  CINIT(ACCEPTTIMEOUT_MS, LONG, 212),
  CINIT(TCP_KEEPALIVE, LONG, 213),
  CINIT(TCP_KEEPIDLE, LONG, 214),
  CINIT(TCP_KEEPINTVL, LONG, 215),
  CINIT(SSL_OPTIONS, LONG, 216),
  CINIT(MAIL_AUTH, STRINGPOINT, 217),
  CINIT(SASL_IR, LONG, 218),
  CINIT(XFERINFOFUNCTION, FUNCTIONPOINT, 219),
  CINIT(XOAUTH2_BEARER, STRINGPOINT, 220),
  CINIT(DNS_INTERFACE, STRINGPOINT, 221),
  CINIT(DNS_LOCAL_IP4, STRINGPOINT, 222),
  CINIT(DNS_LOCAL_IP6, STRINGPOINT, 223),
  CINIT(LOGIN_OPTIONS, STRINGPOINT, 224),
  CINIT(SSL_ENABLE_NPN, LONG, 225),
  CINIT(SSL_ENABLE_ALPN, LONG, 226),
  CINIT(EXPECT_100_TIMEOUT_MS, LONG, 227),
  CINIT(PROXYHEADER, OBJECTPOINT, 228),
  CINIT(HEADEROPT, LONG, 229),
  CINIT(PINNEDPUBLICKEY, STRINGPOINT, 230),
  CINIT(UNIX_SOCKET_PATH, STRINGPOINT, 231),
  CINIT(SSL_VERIFYSTATUS, LONG, 232),
  CINIT(SSL_FALSESTART, LONG, 233),
  CINIT(PATH_AS_IS, LONG, 234),
  CINIT(PROXY_SERVICE_NAME, STRINGPOINT, 235),
  CINIT(SERVICE_NAME, STRINGPOINT, 236),
  CINIT(PIPEWAIT, LONG, 237),
  CINIT(DEFAULT_PROTOCOL, STRINGPOINT, 238),
  CINIT(STREAM_WEIGHT, LONG, 239),
  CINIT(STREAM_DEPENDS, OBJECTPOINT, 240),
  CINIT(STREAM_DEPENDS_E, OBJECTPOINT, 241),
  CINIT(TFTP_NO_OPTIONS, LONG, 242),
  CINIT(CONNECT_TO, OBJECTPOINT, 243),
  CINIT(TCP_FASTOPEN, LONG, 244),
  CINIT(KEEP_SENDING_ON_ERROR, LONG, 245),
  CINIT(PROXY_CAINFO, STRINGPOINT, 246),
  CINIT(PROXY_CAPATH, STRINGPOINT, 247),
  CINIT(PROXY_SSL_VERIFYPEER, LONG, 248),
  CINIT(PROXY_SSL_VERIFYHOST, LONG, 249),
  CINIT(PROXY_SSLVERSION, LONG, 250),
  CINIT(PROXY_TLSAUTH_USERNAME, STRINGPOINT, 251),
  CINIT(PROXY_TLSAUTH_PASSWORD, STRINGPOINT, 252),
  CINIT(PROXY_TLSAUTH_TYPE, STRINGPOINT, 253),
  CINIT(PROXY_SSLCERT, STRINGPOINT, 254),
  CINIT(PROXY_SSLCERTTYPE, STRINGPOINT, 255),
  CINIT(PROXY_SSLKEY, STRINGPOINT, 256),
  CINIT(PROXY_SSLKEYTYPE, STRINGPOINT, 257),
  CINIT(PROXY_KEYPASSWD, STRINGPOINT, 258),
  CINIT(PROXY_SSL_CIPHER_LIST, STRINGPOINT, 259),
  CINIT(PROXY_CRLFILE, STRINGPOINT, 260),
  CINIT(PROXY_SSL_OPTIONS, LONG, 261),
  CINIT(PRE_PROXY, STRINGPOINT, 262),
  CINIT(PROXY_PINNEDPUBLICKEY, STRINGPOINT, 263),
  CINIT(ABSTRACT_UNIX_SOCKET, STRINGPOINT, 264),
  CINIT(SUPPRESS_CONNECT_HEADERS, LONG, 265),
  CINIT(REQUEST_TARGET, STRINGPOINT, 266),
  CINIT(SOCKS5_AUTH, LONG, 267),
  CINIT(SSH_COMPRESSION, LONG, 268),
  CINIT(MIMEPOST, OBJECTPOINT, 269),
  CINIT(TIMEVALUE_LARGE, OFF_T, 270),
  CINIT(HAPPY_EYEBALLS_TIMEOUT_MS, LONG, 271),
  CINIT(RESOLVER_START_FUNCTION, FUNCTIONPOINT, 272),
  CINIT(RESOLVER_START_DATA, OBJECTPOINT, 273),
  CINIT(HAPROXYPROTOCOL, LONG, 274),
  CINIT(DNS_SHUFFLE_ADDRESSES, LONG, 275),
  CINIT(TLS13_CIPHERS, STRINGPOINT, 276),
  CINIT(PROXY_TLS13_CIPHERS, STRINGPOINT, 277),
  CINIT(DISALLOW_USERNAME_IN_URL, LONG, 278),
  CINIT(DOH_URL, STRINGPOINT, 279),
  CINIT(UPLOAD_BUFFERSIZE, LONG, 280),
  CINIT(UPKEEP_INTERVAL_MS, LONG, 281),
  CINIT(CURLU, OBJECTPOINT, 282),
  CINIT(TRAILERFUNCTION, FUNCTIONPOINT, 283),
  CINIT(TRAILERDATA, OBJECTPOINT, 284),
  CINIT(HTTP09_ALLOWED, LONG, 285),
  CINIT(ALTSVC_CTRL, LONG, 286),
  CINIT(ALTSVC, STRINGPOINT, 287),
  CURLOPT_LASTENTRY /* the last unused */
} CURLoption;
#undef LONG
#undef OBJECTPOINT
#undef STRINGPOINT
#undef FUNCTIONPOINT
#undef OFF_T
#undef CINIT

static size_t my_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    x86emu_t *emu = (x86emu_t*)userdata;
    SetCallbackArg(emu, 0, ptr);
    SetCallbackArg(emu, 1, (void*)size);
    SetCallbackArg(emu, 2, (void*)nmemb);
    return RunCallback(emu);
}

static size_t my_read_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
    x86emu_t *emu = (x86emu_t*)userdata;
    SetCallbackArg(emu, 0, buffer);
    SetCallbackArg(emu, 1, (void*)size);
    SetCallbackArg(emu, 2, (void*)nitems);
    return RunCallback(emu);
}

static size_t my_ioctl_callback(void* handle, int32_t fnc, void* userdata)
{
    x86emu_t *emu = (x86emu_t*)userdata;
    SetCallbackArg(emu, 0, handle);
    SetCallbackArg(emu, 1, (void*)fnc);
    return RunCallback(emu);
}

static int32_t my_seek_callback(void* userdata, int64_t off, int32_t origin)
{
    x86emu_t *emu = (x86emu_t*)userdata;
    uint32_t poff;
    poff = (off&0xffffffff);
    SetCallbackArg(emu, 1, (void*)poff);
    poff = ((off>>32)&0xffffffff);
    SetCallbackArg(emu, 2, (void*)poff);
    SetCallbackArg(emu, 3, (void*)origin);
    return (int32_t)RunCallback(emu);
}
/*
static size_t my_header_callback(char* buffer, size_t size, size_t nitems, void* userdata)
{
    x86emu_t *emu = (x86emu_t*)userdata;
    SetCallbackArg(emu, 0, buffer);
    SetCallbackArg(emu, 1, (void*)size);
    SetCallbackArg(emu, 2, (void*)nitems);
    return RunCallback(emu);
}
*/

EXPORT void my_curl_easy_cleanup(x86emu_t* emu, void* handle)
{
    library_t * lib = GetLib(emu->context->maplib, curlName);
    curl_my_t *my = (curl_my_t*)lib->priv.w.p2;

    clean_cdata(my, handle);

    my->curl_easy_cleanup(handle);
}

static curldata_t* getdata(curl_my_t *my, void* handle)
{
    khint_t k;
    int ret;
    k = kh_get(curldata, my->curldata, (uintptr_t)handle);
    if(k==kh_end(my->curldata)) {
        k = kh_put(curldata, my->curldata, (uintptr_t)handle, &ret); 
        memset(&kh_value(my->curldata, k), 0, sizeof(curldata_t));
    }
    return &kh_value(my->curldata, k);
}

EXPORT uint32_t my_curl_easy_setopt(x86emu_t* emu, void* handle, uint32_t option, void* param)
{
    library_t * lib = GetLib(emu->context->maplib, curlName);
    curl_my_t *my = (curl_my_t*)lib->priv.w.p2;
    curldata_t *cdata = getdata(my, handle);
    switch(option) {
        case CURLOPT_WRITEDATA:
            cdata->writedata = param;
            if(cdata->writefnc) {
                SetCallbackArg(cdata->writefnc, 3, param);
                return 0;
            }
            return my->curl_easy_setopt(handle, option, param);
        case CURLOPT_WRITEFUNCTION:
            if(!param) {
                if(cdata->writefnc) {
                    my->curl_easy_setopt(handle, CURLOPT_WRITEDATA, cdata->writedata);
                    FreeCallback(cdata->writefnc);
                }
                cdata->writefnc = NULL;
                return my->curl_easy_setopt(handle, option, param);
            }
            cdata->writefnc = AddCallback(emu, (uintptr_t)param, 4, NULL, NULL, NULL, cdata->writedata);
            my->curl_easy_setopt(handle, CURLOPT_WRITEDATA, cdata->writefnc);
            return my->curl_easy_setopt(handle, option, my_write_callback);
        case CURLOPT_READDATA:
            cdata->readdata = param;
            if(cdata->readfnc) {
                SetCallbackArg(cdata->readfnc, 3, param);
                return 0;
            }
            return my->curl_easy_setopt(handle, option, param);
        case CURLOPT_READFUNCTION:
            if(!param) {
                if(cdata->readfnc) {
                    my->curl_easy_setopt(handle, CURLOPT_READDATA, cdata->readdata);
                    FreeCallback(cdata->readfnc);
                }
                cdata->readfnc = NULL;
                return my->curl_easy_setopt(handle, option, param);
            }
            cdata->readfnc = AddCallback(emu, (uintptr_t)param, 4, NULL, NULL, NULL, cdata->readdata);
            my->curl_easy_setopt(handle, CURLOPT_READDATA, cdata->readfnc);
            return my->curl_easy_setopt(handle, option, my_read_callback);
        case CURLOPT_IOCTLDATA:
            cdata->iodata = param;
            if(cdata->iofnc) {
                SetCallbackArg(cdata->iofnc, 2, param);
                return 0;
            }
            return my->curl_easy_setopt(handle, option, param);
        case CURLOPT_IOCTLFUNCTION:
            if(!param) {
                if(cdata->iofnc) {
                    my->curl_easy_setopt(handle, CURLOPT_IOCTLDATA, cdata->iodata);
                    FreeCallback(cdata->iofnc);
                }
                cdata->iofnc = NULL;
                return my->curl_easy_setopt(handle, option, param);
            }
            cdata->iofnc = AddCallback(emu, (uintptr_t)param, 3, NULL, NULL, cdata->iodata, NULL);
            my->curl_easy_setopt(handle, CURLOPT_IOCTLDATA, cdata->iofnc);
            return my->curl_easy_setopt(handle, option, my_ioctl_callback);
        case CURLOPT_SEEKDATA:
            cdata->seekdata = param;
            if(cdata->seekfnc) {
                SetCallbackArg(cdata->seekfnc, 0, param);
                return 0;
            }
            return my->curl_easy_setopt(handle, option, param);
        case CURLOPT_SEEKFUNCTION:
            if(!param) {
                if(cdata->seekfnc) {
                    my->curl_easy_setopt(handle, CURLOPT_SEEKDATA, cdata->seekdata);
                    FreeCallback(cdata->seekfnc);
                }
                cdata->seekfnc = NULL;
                return my->curl_easy_setopt(handle, option, param);
            }
            cdata->seekfnc = AddCallback(emu, (uintptr_t)param, 4, cdata->seekdata, NULL, NULL, NULL); // because offset if 64bits
            my->curl_easy_setopt(handle, CURLOPT_SEEKDATA, cdata->iofnc);
            return my->curl_easy_setopt(handle, option, my_seek_callback);
        case CURLOPT_HEADERDATA:
            cdata->headerdata = param;
            if(cdata->headerfnc) {
                SetCallbackArg(cdata->headerfnc, 3, param);
                return 0;
            }
            return my->curl_easy_setopt(handle, option, param);
        case CURLOPT_HEADERFUNCTION:
            if(!param) {
                if(cdata->headerfnc) {
                    my->curl_easy_setopt(handle, CURLOPT_HEADERDATA, cdata->headerdata);
                    FreeCallback(cdata->headerfnc);
                }
                cdata->headerfnc = NULL;
                return my->curl_easy_setopt(handle, option, param);
            }
            cdata->headerfnc = AddCallback(emu, (uintptr_t)param, 4, NULL, NULL, NULL, cdata->headerdata);
            my->curl_easy_setopt(handle, CURLOPT_HEADERDATA, cdata->headerfnc);
            return my->curl_easy_setopt(handle, option, my_read_callback);
        case CURLOPT_SOCKOPTFUNCTION:
        case CURLOPT_SOCKOPTDATA:
        case CURLOPT_OPENSOCKETFUNCTION:
        case CURLOPT_OPENSOCKETDATA:
        case CURLOPT_CLOSESOCKETFUNCTION:
        case CURLOPT_CLOSESOCKETDATA:
        case CURLOPT_PROGRESSFUNCTION:
        case CURLOPT_PROGRESSDATA:
        case CURLOPT_XFERINFOFUNCTION:
        //case CURLOPT_XFERINFODATA:
        case CURLOPT_DEBUGFUNCTION:
        case CURLOPT_DEBUGDATA:
        case CURLOPT_SSL_CTX_FUNCTION:
        case CURLOPT_SSL_CTX_DATA:
        case CURLOPT_CONV_TO_NETWORK_FUNCTION:
        case CURLOPT_CONV_FROM_NETWORK_FUNCTION:
        case CURLOPT_CONV_FROM_UTF8_FUNCTION:
        case CURLOPT_INTERLEAVEFUNCTION:
        case CURLOPT_INTERLEAVEDATA:
        case CURLOPT_CHUNK_BGN_FUNCTION:
        case CURLOPT_CHUNK_END_FUNCTION:
        case CURLOPT_CHUNK_DATA:
        case CURLOPT_FNMATCH_FUNCTION:
        case CURLOPT_FNMATCH_DATA:
        case CURLOPT_SUPPRESS_CONNECT_HEADERS:
        case CURLOPT_RESOLVER_START_FUNCTION:
        case CURLOPT_RESOLVER_START_DATA:
            printf_log(LOG_NONE, "Error: unimplemented option %u in curl_easy_setopt\n", option);
            return 48; //unknown option...
        default:
            return my->curl_easy_setopt(handle, option, param);
    }
}


#define CUSTOM_INIT \
    lib->priv.w.p2 = getCurlMy(lib);

#define CUSTOM_FINI \
    freeCurlMy(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

