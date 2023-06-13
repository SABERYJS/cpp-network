#ifndef MT_HTTP_HEADER_TYPE_H
#define MT_HTTP_HEADER_TYPE_H

//all http request headers
//reference https://datatracker.ietf.org/doc/html/rfc2616#section-5.3
//reference https://datatracker.ietf.org/doc/html/rfc2616#autoid-163
enum HttpHeaderTypes {
    Unknown,
    Custom, //stand for custom header
    Accept,
    AcceptCharset,
    AcceptEncoding,
    AcceptLanguage,
    AcceptRanges,
    Authorization,
    Age,
    Allow,
    CacheControl,
    Connection,
    ContenEncode,
    ContentLanguage,
    ContentLength,
    ContentLocation,
    ContentMd5,
    ContentRange,
    ContentType,
    Date,
    ETag,
    Expect,
    Expires,
    From,
    Host,
    IfMatch,
    IfModifiedSince,
    IfNoneMatch,
    IfRange,
    IfUnmodifiedSince,
    LastModified,
    MaxForwards,
    Pragma,
    ProxyAuthenticate,
    ProxyAuthorization,
    Range,
    Referer,
    RetryAfter,
    Server,
    TE,
    Trailer,
    TransferEncoding,
    Upgrade,
    UserAgent,
    Vari,
    Via,
    Warning,
    WWWAuthenticate
};


static const char* ContentLengthConst = "Content-Length";
static const char* TransferEncodingConst = "Transfer-Encoding";
#endif