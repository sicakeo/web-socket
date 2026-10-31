#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Callback function to handle WebSocket events
static int callback_file_transfer(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("Client connected.\n");
            break;
            
        case LWS_CALLBACK_RECEIVE:
            // Open file in append-binary mode to handle incoming encrypted chunks
            FILE *fp = fopen("received_encrypted_file.dat", "ab");
            if (fp) {
                fwrite(in, 1, len, fp);
                fclose(fp);
                printf("Received and wrote %zu bytes.\n", len);
            } else {
                printf("Failed to open file for writing.\n");
            }
            break;
            
        case LWS_CALLBACK_CLOSED:
            printf("Client disconnected.\n");
            break;
            
        default:
            break;
    }
    return 0;
}

// Define the supported protocols
static struct lws_protocols protocols[] = {
    {
        "file-transfer-protocol", // Protocol name to match in the browser JS
        callback_file_transfer,   // Callback function
        0,                        // Per-session data size
        65536,                    // Maximum frame size
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

int main(void) {
    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    
    // Server configuration
    info.port = 8443;
    info.protocols = protocols;
    
    // SSL Configuration using your generated files
    info.ssl_cert_filepath = "cert.pem";
    info.ssl_private_key_filepath = "key.pem";
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    // Create the WebSocket context
    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "libwebsockets init failed\n");
        return -1;
    }

    printf("Secure WebSocket server started on port %d...\n", info.port);

    // Infinite event loop to process WebSocket traffic
    while (1) {
        lws_service(context, 1000);
    }

    lws_context_destroy(context);
    return 0;
}
