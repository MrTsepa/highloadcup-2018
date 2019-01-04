#include <evhttp.h>
#include <err.h>
#include <string>


void testing(struct evhttp_request *request, void *privParams) {
    struct evbuffer *buffer;
    struct evkeyvalq headers;
    const char *q;
    // Parse the query for later lookups
    evhttp_parse_query (evhttp_request_get_uri (request), &headers);

    // lookup the 'q' GET parameter
    q = evhttp_find_header (&headers, "q");

    // Create an answer buffer where the data to send back to the browser will be appened
    buffer = evbuffer_new ();
    evbuffer_add (buffer, "coucou !", 8);
    evbuffer_add_printf (buffer, "%s", q);

    // Add a HTTP header, an application/json for the content type here
    evhttp_add_header (evhttp_request_get_output_headers (request),
                       "Content-Type", "text/plain");

    // Tell we're done and data should be sent back
    evhttp_send_reply(request, HTTP_OK, "OK", buffer);

    // Free up stuff
    evhttp_clear_headers (&headers);

    evbuffer_free (buffer);

    return;
}


void notfound(struct evhttp_request *request, void *params) {
    evhttp_send_error(request, HTTP_NOTFOUND, "Not Found");
}

int main() {
    struct event_base *ebase;
    struct evhttp *server;

    // Create a new event handler
    ebase = event_base_new ();;

    // Create a http server using that handler
    server = evhttp_new (ebase);

    // Set a test callback, /testing
    evhttp_set_cb (server, "/testing", testing, nullptr);

    // Set the callback for anything not recognized
    evhttp_set_gencb (server, notfound, nullptr);

    // Listen locally on port 32001
    if (evhttp_bind_socket (server, "127.0.0.1", 8080) != 0)
        errx(1, "Could not bind to 127.0.0.1:8080");

    // Start processing queries
    event_base_dispatch(ebase);

    // Free up stuff
    evhttp_free (server);

    event_base_free (ebase);
}