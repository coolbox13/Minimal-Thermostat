#ifdef HTTP_METHOD_CONFLICT_FIXED
  #ifndef _ESPAsyncWebServer_H_  // Only redefine if ESPAsyncWebServer hasn't been included yet.
    #undef HTTP_DELETE
    #undef HTTP_GET  
    #undef HTTP_HEAD
    #undef HTTP_POST
    #undef HTTP_PUT
    #undef HTTP_OPTIONS
    #undef HTTP_PATCH
    
    #define HTTP_GET     ((1<<0))
    #define HTTP_POST    ((1<<1))
    #define HTTP_DELETE  ((1<<2))
    #define HTTP_PUT     ((1<<3))
    #define HTTP_PATCH   ((1<<4))
    #define HTTP_HEAD    ((1<<5))
    #define HTTP_OPTIONS ((1<<6))
  #endif
#endif