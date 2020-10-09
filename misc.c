#include "misc.h"

#include "posix_naos.h"
#include "posix_string.h"

int misc_build_endpoint(const char *str, struct misc_endpoint *endpoint)
{
    char *dup;
    DDN_IPV4(host);
    int port;
    int i;
    char *cursor;
    int is_endpoint_delimiter_found;
    char segm[4], segp[6];
    int j;
    int retval;
    int k;

    i = 0;
    port = 0;
    is_endpoint_delimiter_found = 0;
    j = 0;
    retval = 0;
    k = 0;
    memset(segm, 0, sizeof(segm));
    memset(host, 0, sizeof(host));
    memset(segp, 0, sizeof(segp));

    dup = posix__strdup(str);
    cursor = dup;
    while (*cursor) {
        if (':' == *cursor) {

            /* 192.168.1 */
            if (k != 3) {
                retval = -1;
                break;
            }

            is_endpoint_delimiter_found = 1;
            cursor++;
            j = 0;
            continue;
        }

        if (!is_endpoint_delimiter_found) {  /* analyze the IP address section */

            if (15 == i) {
                retval = -1;
                break;
            }

            /* 192.168.1.0.1 */
            if (k > 3) {
                retval = -1;
                break;
            }

            if (*cursor == '.' ) {
                /* .192.168.2.2 */
                if (0 == segm[0]) {
                    retval = -1;
                    break;
                }

                /* 256.1.1.0 */
                if (atoi(segm) > 255) {
                    retval = -1;
                    break;
                }

                host[i] = *cursor;
                memset(segm, 0, sizeof(segm));
                cursor++;
                i++;
                k++;
                j = 0;
                continue;
            }

            /* 1922.0.0.1 */
            if (*cursor >= '0' && *cursor <= '9' ) {
                if (j >= 3) {
                    retval = -1;
                    break;
                }
                segm[j] = *cursor;
                host[i] = *cursor;
                cursor++;
                j++;
                i++;
                continue;
            }

            /* any other characters */
            retval = -1;
            break;

        } else {  /* analyze the port section */

            if (*cursor >= '0' && *cursor <= '9' ) {

                /* :012345 */
                if (0 == j && *cursor == '0') {
                    retval = -1;
                    break;
                }

                /* :123456 */
                if (j >= 5) {
                    retval = -1;
                    break;
                }

                segp[j] = *cursor;
                cursor++;
                i++;
                j++;
                continue;
            }

            /* any other characters */
            retval = -1;
            break;
        }
    }

    if (!is_endpoint_delimiter_found) {
        retval = -1;
    }

    /* no port specify */
    if ( retval >= 0 && endpoint ) {
        strcpy(endpoint->host, host);
        port = atoi(segp);
        endpoint->inet = posix__ipv4tou(endpoint->host, kByteOrder_LittleEndian);
        if (port > 65535) {
            retval = -1;
        } else {
            endpoint->port = port;
        }
    }

    free(dup);
    return retval;
}

struct misc_endpoint * misc_duplicate_endpoint(struct misc_endpoint *duplicate, const struct misc_endpoint *source)
{
    assert(source && duplicate);
    duplicate->inet = source->inet;
    duplicate->port = source->port;
    posix__strcpy(duplicate->host, sizeof(duplicate->host), source->host);
    return duplicate;
}
