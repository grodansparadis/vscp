/*!
 *
 * Code from: https://wiki.openssl.org/index.php/SSL/TLS_Client
 * 
 * Get certificats from server
 *      https://superuser.com/questions/97201/how-to-save-a-remote-server-ssl-certificate-locally-as-a-file
 * openssl s_client -showcerts -connect server.edu:443 </dev/null 2>/dev/null|openssl x509 -outform PEM >mycertfile.pem
 *
 * */


#include "simple_tls_client.h"

// uncomment to connect to random.org
// If defined connet to local server with local certs generated
// with generate_cocal_cert (simple_tls_server)
#define LOCAL_TEST

int verify_callback(int preverify, X509_STORE_CTX* x509_ctx);

void init_openssl_library(void);
void print_cn_name(const char* label, X509_NAME* const name);
void print_san_name(const char* label, X509* const cert);
void print_error_string(unsigned long err, const char* const label);

/* Cipher suites, https://www.openssl.org/docs/apps/ciphers.html */
//const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS";

#if 1
const char* const PREFERRED_CIPHERS = "kEECDH:kEDH:kRSA:AESGCM:AES256:AES128:3DES:SHA256:SHA84:SHA1:!aNULL:!eNULL:!EXP:!LOW:!MEDIUM!ADH:!AECDH";
#endif

#if 0
const char* const PREFERRED_CIPHERS = NULL;
#endif

#if 0
const char* PREFERRED_CIPHERS =

/* TLS 1.2 only */
"ECDHE-ECDSA-AES256-GCM-SHA384:"
"ECDHE-RSA-AES256-GCM-SHA384:"
"ECDHE-ECDSA-AES128-GCM-SHA256:"
"ECDHE-RSA-AES128-GCM-SHA256:"

/* TLS 1.2 only */
"DHE-DSS-AES256-GCM-SHA384:"
"DHE-RSA-AES256-GCM-SHA384:"
"DHE-DSS-AES128-GCM-SHA256:"
"DHE-RSA-AES128-GCM-SHA256:"

/* TLS 1.0 only */
"DHE-DSS-AES256-SHA:"
"DHE-RSA-AES256-SHA:"
"DHE-DSS-AES128-SHA:"
"DHE-RSA-AES128-SHA:"

/* SSL 3.0 and TLS 1.0 */
"EDH-DSS-DES-CBC3-SHA:"
"EDH-RSA-DES-CBC3-SHA:"
"DH-DSS-DES-CBC3-SHA:"
"DH-RSA-DES-CBC3-SHA";
#endif

int main(int argc, char* argv[])
{
    UNUSED(argc); UNUSED(argv);
    
#if !defined(NDEBUG)
    //InstallDebugTrapHandler();
#endif
    
    long res = 1;
    int ret = 1;
    unsigned long ssl_err = 0;
    
    SSL_CTX* ctx = NULL;
    BIO *web = NULL, *out = NULL;
    SSL *ssl = NULL;
    
    do {
        
        /* Internal function that wraps the OpenSSL init's   */
        /* Cannot fail because no OpenSSL function fails ??? */
        init_openssl_library();
        
        /* https://www.openssl.org/docs/ssl/SSL_CTX_new.html */
        const SSL_METHOD* method = SSLv23_method();
        //const SSL_METHOD* method = TLSv1_method();
        ssl_err = ERR_get_error();
        
        ASSERT(NULL != method);
        if (!(NULL != method))
        {
            print_error_string(ssl_err, "SSLv23_method");
            break; /* failed */
        }
        
        /* http://www.openssl.org/docs/ssl/ctx_new.html */
        ctx = SSL_CTX_new(method);
        /* ctx = SSL_CTX_new(TLSv1_method()); */
        ssl_err = ERR_get_error();
        
        ASSERT(ctx != NULL);
        if(!(ctx != NULL))
        {
            print_error_string(ssl_err, "SSL_CTX_new");
            break; /* failed */
        }
        
        /* https://www.openssl.org/docs/ssl/ctx_set_verify.html */
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
        /* Cannot fail ??? */
        
        /* https://www.openssl.org/docs/ssl/ctx_set_verify.html */
        SSL_CTX_set_verify_depth(ctx, 5);
        /* Cannot fail ??? */
        
        /* Remove the most egregious. Because SSLv2 and SSLv3 have been      */
        /* removed, a TLSv1.0 handshake is used. The client accepts TLSv1.0  */
        /* and above. An added benefit of TLS 1.0 and above are TLS          */
        /* extensions like Server Name Indicatior (SNI).                     */
        const long flags = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
        long old_opts = SSL_CTX_set_options(ctx, flags);
        UNUSED(old_opts);
        
        /* http://www.openssl.org/docs/ssl/SSL_CTX_load_verify_locations.html */
        //res = SSL_CTX_load_verify_locations(ctx, "random-org-chain.pem", NULL);
        res = SSL_CTX_load_verify_locations(ctx, "cert.pem", NULL);
        ssl_err = ERR_get_error();
        
        //ASSERT(1 == res);
        if( !(1 == res) )
        {
            /* Non-fatal, but something else will probably break later */
            print_error_string( ssl_err, "SSL_CTX_load_verify_locations");
            /* break; */
        }
        
        /* https://www.openssl.org/docs/crypto/BIO_f_ssl.html */
        web = BIO_new_ssl_connect(ctx);
        ssl_err = ERR_get_error();
        
        ASSERT(web != NULL);
        if(!(web != NULL))
        {
            print_error_string(ssl_err, "BIO_new_ssl_connect");
            break; /* failed */
        }
        
        /* https://www.openssl.org/docs/crypto/BIO_s_connect.html */
        //res = BIO_set_conn_hostname(web, HOST_NAME ":" HOST_PORT);
        res = BIO_set_conn_hostname(web,  "localhost:4433");
        ssl_err = ERR_get_error();
        
        ASSERT(1 == res);
        if(!(1 == res))
        {
            print_error_string(ssl_err, "BIO_set_conn_hostname");
            break; /* failed */
        }
        
        /* https://www.openssl.org/docs/crypto/BIO_f_ssl.html */
        /* This copies an internal pointer. No need to free.  */
        BIO_get_ssl(web, &ssl);
        ssl_err = ERR_get_error();
        
        ASSERT(ssl != NULL);
        if(!(ssl != NULL))
        {
            print_error_string(ssl_err, "BIO_get_ssl");
            break; /* failed */
        }
        
        /* https://www.openssl.org/docs/ssl/ssl.html#DEALING_WITH_PROTOCOL_CONTEXTS */
        /* https://www.openssl.org/docs/ssl/SSL_CTX_set_cipher_list.html            */
        res = SSL_set_cipher_list(ssl, PREFERRED_CIPHERS);
        ssl_err = ERR_get_error();
        
        ASSERT(1 == res);
        if(!(1 == res))
        {
            print_error_string(ssl_err, "SSL_set_cipher_list");
            break; /* failed */
        }

        /* No documentation. See the source code for tls.h and s_client.c */
        res = SSL_set_tlsext_host_name(ssl, HOST_NAME);
        ssl_err = ERR_get_error();
        
        ASSERT(1 == res);
        if(!(1 == res))
        {
            /* Non-fatal, but who knows what cert might be served by an SNI server  */
            /* (We know its the default site's cert in Apache and IIS...)           */
            print_error_string(ssl_err, "SSL_set_tlsext_host_name");
            /* break; */
        }
        
        /* https://www.openssl.org/docs/crypto/BIO_s_file.html */
        out = BIO_new_fp(stdout, BIO_NOCLOSE);
        ssl_err = ERR_get_error();
        
        ASSERT(NULL != out);
        if(!(NULL != out))
        {
            print_error_string(ssl_err, "BIO_new_fp");
            break; /* failed */
        }
        
        /* https://www.openssl.org/docs/crypto/BIO_s_connect.html */
        res = BIO_do_connect(web);
        ssl_err = ERR_get_error();
        
        ASSERT(1 == res);
        if(!(1 == res))
        {
            print_error_string(ssl_err, "BIO_do_connect");
            break; /* failed */
        }
        
        /* https://www.openssl.org/docs/crypto/BIO_f_ssl.html */
        res = BIO_do_handshake(web);
        ssl_err = ERR_get_error();
        
        ASSERT(1 == res);
        if(!(1 == res))
        {
            print_error_string(ssl_err, "BIO_do_handshake");
            break; /* failed */
        }
        
        /**************************************************************************************/
        /**************************************************************************************/
        /* You need to perform X509 verification here. There are two documents that provide   */
        /*   guidance on the gyrations. First is RFC 5280, and second is RFC 6125. Two other  */
        /*   documents of interest are:                                                       */
        /*     Baseline Certificate Requirements:                                             */
        /*       https://www.cabforum.org/Baseline_Requirements_V1_1_6.pdf                    */
        /*     Extended Validation Certificate Requirements:                                  */
        /*       https://www.cabforum.org/Guidelines_v1_4_3.pdf                               */
        /*                                                                                    */
        /* Here are the minimum steps you should perform:                                     */
        /*   1. Call SSL_get_peer_certificate and ensure the certificate is non-NULL. It      */
        /*      should never be NULL because Anonymous Diffie-Hellman (ADH) is not allowed.   */
        /*   2. Call SSL_get_verify_result and ensure it returns X509_V_OK. This return value */
        /*      depends upon your verify_callback if you provided one. If not, the library    */
        /*      default validation is fine (and you should not need to change it).            */
        /*   3. Verify either the CN or the SAN matches the host you attempted to connect to. */
        /*      Note Well (N.B.): OpenSSL prior to version 1.1.0 did *NOT* perform hostname   */
        /*      verification. If you are using OpenSSL 0.9.8 or 1.0.1, then you will need     */
        /*      to perform hostname verification yourself. The code to get you started on     */
        /*      hostname verification is provided in print_cn_name and print_san_name. Be     */
        /*      sure you are sensitive to ccTLDs (don't navively transform the hostname       */
        /*      string). http://publicsuffix.org/ might be helpful.                           */
        /*                                                                                    */
        /* If all three checks succeed, then you have a chance at a secure connection. But    */
        /*   its only a chance, and you should either pin your certificates (to remove DNS,   */
        /*   CA, and Web Hosters from the equation) or implement a Trust-On-First-Use (TOFU)  */
        /*   scheme like Perspectives or SSH. But before you TOFU, you still have to make     */
        /*   the customary checks to ensure the certifcate passes the sniff test.             */
        /*                                                                                    */
        /* Happy certificate validation hunting!                                              */
        /**************************************************************************************/
        /**************************************************************************************/
        
        
        /* Step 1: verify a server certifcate was presented during negotiation */
        /* https://www.openssl.org/docs/ssl/SSL_get_peer_certificate.html          */
        X509* cert = SSL_get_peer_certificate(ssl);
        if(cert) { X509_free(cert); } /* Free immediately */
        
        ASSERT(NULL != cert);
        if(NULL == cert)
        {
            /* Hack a code for print_error_string. */
            print_error_string(X509_V_ERR_APPLICATION_VERIFICATION, "SSL_get_peer_certificate");
            break; /* failed */
        }
        
        /* Step 2: verify the result of chain verifcation             */
        /* http://www.openssl.org/docs/ssl/SSL_get_verify_result.html */
        /* Error codes: http://www.openssl.org/docs/apps/verify.html  */
        res = SSL_get_verify_result(ssl);
        
        ASSERT(X509_V_OK == res);
        if(!(X509_V_OK == res))
        {
            /* Hack a code into print_error_string. */
            print_error_string((unsigned long)res, "SSL_get_verify_results");
            break; /* failed */
        }
        
        /* Step 3: hostname verifcation.   */
        /* An exercise left to the reader. */
        
        /**************************************************************************************/
        /**************************************************************************************/
        /* Now, we can finally start reading and writing to the BIO...                        */
        /**************************************************************************************/
        /**************************************************************************************/
        
        BIO_puts(web, "GET " HOST_RESOURCE " HTTP/1.1\r\nHost: " HOST_NAME "\r\nConnection: close\r\n\r\n");
        BIO_puts(out, "\nFetching: " HOST_RESOURCE "\n\n");
        
        int len = 0;
        do {
            char buff[1536] = {};
            
            /* https://www.openssl.org/docs/crypto/BIO_read.html */
            len = BIO_read(web, buff, sizeof(buff));
            
            if(len > 0)
                BIO_write(out, buff, len);
            
            /* BIO_should_retry returns TRUE unless there's an  */
            /* error. We expect an error when the server        */
            /* provides the response and closes the connection. */
            
        } while (len > 0 || BIO_should_retry(web));
        
        ret = 0;
        
    } while (0);
    
    if(out)
        BIO_free(out);
    
    if(web != NULL)
        BIO_free_all(web);
    
    if(NULL != ctx)
        SSL_CTX_free(ctx);
    
    return ret;
}

void init_openssl_library(void)
{
    /* https://www.openssl.org/docs/ssl/SSL_library_init.html */
    (void)SSL_library_init();
    /* Cannot fail (always returns success) ??? */
    
    /* https://www.openssl.org/docs/crypto/ERR_load_crypto_strings.html */
    SSL_load_error_strings();
    /* Cannot fail ??? */
    
    /* SSL_load_error_strings loads both libssl and libcrypto strings */
    /* ERR_load_crypto_strings(); */
    /* Cannot fail ??? */
    
    /* OpenSSL_config may or may not be called internally, based on */
    /*  some #defines and internal gyrations. Explicitly call it    */
    /*  *IF* you need something from openssl.cfg, such as a         */
    /*  dynamically configured ENGINE.                              */
    OPENSSL_config(NULL);
    /* Cannot fail ??? */
    
    /* Include <openssl/opensslconf.h> to get this define     */
#if defined (OPENSSL_THREADS)
    /* TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO */
    /* https://www.openssl.org/docs/crypto/threads.html */
    fprintf(stdout, "Warning: thread locking is not implemented\n");
#endif
}

void print_cn_name(const char* label, X509_NAME* const name)
{
    int idx = -1, success = 0;
    unsigned char *utf8 = NULL;
    
    do
    {
        if(!name) break; /* failed */
        
        idx = X509_NAME_get_index_by_NID(name, NID_commonName, -1);
        if(!(idx > -1))  break; /* failed */
        
        X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, idx);
        if(!entry) break; /* failed */
        
        ASN1_STRING* data = X509_NAME_ENTRY_get_data(entry);
        if(!data) break; /* failed */
        
        int length = ASN1_STRING_to_UTF8(&utf8, data);
        if(!utf8 || !(length > 0))  break; /* failed */
        
        fprintf(stdout, "  %s: %s\n", label, utf8);
        success = 1;
        
    } while (0);
    
    if(utf8)
        OPENSSL_free(utf8);
    
    if(!success)
        fprintf(stdout, "  %s: <not available>\n", label);
}

void print_san_name(const char* label, X509* const cert)
{
    int success = 0;
    GENERAL_NAMES* names = NULL;
    unsigned char* utf8 = NULL;
    
    do
    {
        if(!cert) break; /* failed */
        
        names = X509_get_ext_d2i(cert, NID_subject_alt_name, 0, 0 );
        if(!names) break;
        
        int i = 0, count = sk_GENERAL_NAME_num(names);
        if(!count) break; /* failed */
        
        for( i = 0; i < count; ++i )
        {
            GENERAL_NAME* entry = sk_GENERAL_NAME_value(names, i);
            if(!entry) continue;
            
            if(GEN_DNS == entry->type)
            {
                int len1 = 0, len2 = -1;
                
                len1 = ASN1_STRING_to_UTF8(&utf8, entry->d.dNSName);
                if(utf8) {
                    len2 = (int)strlen((const char*)utf8);
                }
                
                if(len1 != len2) {
                    fprintf(stderr, "  Strlen and ASN1_STRING size do not match (embedded null?): %d vs %d\n", len2, len1);
                }
                
                /* If there's a problem with string lengths, then     */
                /* we skip the candidate and move on to the next.     */
                /* Another policy would be to fails since it probably */
                /* indicates the client is under attack.              */
                if(utf8 && len1 && len2 && (len1 == len2)) {
                    fprintf(stdout, "  %s: %s\n", label, utf8);
                    success = 1;
                }
                
                if(utf8) {
                    OPENSSL_free(utf8), utf8 = NULL;
                }
            }
            else
            {
                fprintf(stderr, "  Unknown GENERAL_NAME type: %d\n", entry->type);
            }
        }

    } while (0);
    
    if(names)
        GENERAL_NAMES_free(names);
    
    if(utf8)
        OPENSSL_free(utf8);
    
    if(!success)
        fprintf(stdout, "  %s: <not available>\n", label);
    
}

int verify_callback(int preverify, X509_STORE_CTX* x509_ctx)
{
    /* For error codes, see http://www.openssl.org/docs/apps/verify.html  */
    
    int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
    int err = X509_STORE_CTX_get_error(x509_ctx);
    
    X509* cert = X509_STORE_CTX_get_current_cert(x509_ctx);
    X509_NAME* iname = cert ? X509_get_issuer_name(cert) : NULL;
    X509_NAME* sname = cert ? X509_get_subject_name(cert) : NULL;
    
    fprintf(stdout, "verify_callback (depth=%d)(preverify=%d)\n", depth, preverify);
    
    /* Issuer is the authority we trust that warrants nothing useful */
    print_cn_name("Issuer (cn)", iname);
    
    /* Subject is who the certificate is issued to by the authority  */
    print_cn_name("Subject (cn)", sname);
    
    if(depth == 0) {
        /* If depth is 0, its the server's certificate. Print the SANs */
        print_san_name("Subject (san)", cert);
    }
    
    if ( preverify == 0 )
    {
        if(err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
            fprintf(stdout, "  Error = X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY\n");
        else if(err == X509_V_ERR_CERT_UNTRUSTED)
            fprintf(stdout, "  Error = X509_V_ERR_CERT_UNTRUSTED\n");
        else if(err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)
            fprintf(stdout, "  Error = X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN\n");
        else if(err == X509_V_ERR_CERT_NOT_YET_VALID)
            fprintf(stdout, "  Error = X509_V_ERR_CERT_NOT_YET_VALID\n");
        else if(err == X509_V_ERR_CERT_HAS_EXPIRED)
            fprintf(stdout, "  Error = X509_V_ERR_CERT_HAS_EXPIRED\n");
        else if(err == X509_V_OK)
            fprintf(stdout, "  Error = X509_V_OK\n");
        else
            fprintf(stdout, "  Error = %d\n", err);
    }

#if !defined(NDEBUG)
    return 1;
#else
    return preverify;
#endif
}

void print_error_string(unsigned long err, const char* const label)
{
    const char* const str = ERR_reason_error_string(err);
    if(str)
        fprintf(stderr, "%s\n", str);
    else
        fprintf(stderr, "%s failed: %lu (0x%lx)\n", label, err, err);
}
