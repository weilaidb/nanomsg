/*
    Copyright (c) 2013 Martin Sustrik  All rights reserved.
    Copyright 2016 Franklin "Snaipe" Mathieu <franklinmathieu@gmail.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "../src/nn.h"
#include "../src/pair.h"

#include "testutil.h"
#include "../src/mlog/mlog_init.h"

#include <string.h>

#define SOCKET_ADDRESS "inproc://a"

char longdata[1 << 20];

int main (int argc, const char *argv[])
{
    mlog_byfunc("begin ~~~");
    
    int rc;
    int sb;
    int sc;
    unsigned char *buf1, *buf2;
    int i;
    struct nn_iovec iov;
    struct nn_msghdr hdr;
    char socket_address_tcp[128] = {0};
    mlog_msgbyfunc(socket_address_tcp,sizeof(socket_address_tcp),"socket_address_tcp");

    test_addr_from(socket_address_tcp, "tcp", "127.0.0.1",
            get_test_port(argc, argv));
    mlog_byfunc("socket_address_tcp:%s ~~~", socket_address_tcp);
    mlog_byfunc("port:%u ~~~", get_test_port(argc, argv));
    /*  ??sb ??sc???????SOCKET_ADDRESS  */

    sb = test_socket (AF_SP, NN_PAIR);
    mlog_byfunc("sb:%u ~~~", sb);
    test_bind (sb, SOCKET_ADDRESS);
    mlog_byfunc("SOCKET_ADDRESS:%s", SOCKET_ADDRESS);
    sc = test_socket (AF_SP, NN_PAIR);
    mlog_byfunc("sc:%u ~~~", sc);
    test_connect (sc, SOCKET_ADDRESS);
    mlog_byfunc("SOCKET_ADDRESS2:%s", SOCKET_ADDRESS);

    buf1 = nn_allocmsg (1024, 0);
	mlog_byfunc("allocate buf1 address:%p", buf1);
    mlog_msgbyfunc(buf1,1024,"nn_allocmsg buff");
    alloc_assert (buf1);
    for (i = 0; i != 1024; ++i)
        buf1 [i] = (unsigned char) i;

    mlog_msgbyfunc(buf1,1024,"after set nn_allocmsg buff1, then send by sc");
    
    mlog_msgbyfunc(buf1,1024,"buf1 :    %p   ", buf1);
    rc = nn_send (sc, &buf1, NN_MSG, 0);
    errno_assert (rc >= 0);
    mlog_byfunc("rc:%u", rc);
    nn_assert (rc == 1024);
	
	
    buf2 = NULL;
    rc = nn_recv (sb, &buf2, NN_MSG, 0);
    errno_assert (rc >= 0);
    nn_assert (rc == 1024);
    mlog_byfunc("buf2 rc:%u", rc);
    mlog_msgbyfunc(buf2,1024,"sb receive buf2");
	mlog_byfunc("receive buf2 address:%p", buf2);
    nn_assert (buf2);
    for (i = 0; i != 1024; ++i)
        nn_assert (buf2 [i] == (unsigned char) i);
    rc = nn_freemsg (buf2);
    errno_assert (rc == 0);

    buf1 = nn_allocmsg (1024, 0);
    alloc_assert (buf1);
    for (i = 0; i != 1024; ++i)
        buf1 [i] = (unsigned char) i;
    iov.iov_base = &buf1;
    iov.iov_len = NN_MSG;
	mlog_byfunc("NN_MSG:%u", NN_MSG);
    memset (&hdr, 0, sizeof (hdr));
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    rc = nn_sendmsg (sc, &hdr, 0);
    errno_assert (rc >= 0);
    nn_assert (rc == 1024);

    buf2 = NULL;
    iov.iov_base = &buf2;
    iov.iov_len = NN_MSG;
	
    memset (&hdr, 0, sizeof (hdr));
    hdr.msg_iov = &iov;
    hdr.msg_iovlen = 1;
    rc = nn_recvmsg (sb, &hdr, 0);
    errno_assert (rc >= 0);
    nn_assert (rc == 1024);
    nn_assert (buf2);
    for (i = 0; i != 1024; ++i)
        nn_assert (buf2 [i] == (unsigned char) i);
    rc = nn_freemsg (buf2);
    errno_assert (rc == 0);

    test_close (sc);
    test_close (sb);

    /*  Test receiving of large message  */
	mlog_byfunc(" Test receiving of large message");
	
    sb = test_socket (AF_SP, NN_PAIR);
    test_bind (sb, socket_address_tcp);
    sc = test_socket (AF_SP, NN_PAIR);
    test_connect (sc, socket_address_tcp);

    for (i = 0; i < (int) sizeof (longdata); ++i)
        longdata[i] = '0' + (i % 10);
    longdata [sizeof (longdata) - 1] = 0;
    mlog_msgbyfunc(longdata,sizeof(longdata),"send longdata");
    test_send (sb, longdata);

    rc = nn_recv (sc, &buf2, NN_MSG, 0);
    mlog_msgbyfunc(buf2,sizeof(longdata),"receive  buf2");
    errno_assert (rc >= 0);
    nn_assert (rc == sizeof (longdata) - 1);
    nn_assert (buf2);
    for (i = 0; i < (int) sizeof (longdata) - 1; ++i)
        nn_assert (buf2 [i] == longdata [i]);
    rc = nn_freemsg (buf2);
    errno_assert (rc == 0);

    test_close (sc);
    test_close (sb);


    mlog_byfunc("~~~end ~~~");
    

    showmlogkeys();
    showmlogbyname(__FUNCTION__);

    return 0;
}

