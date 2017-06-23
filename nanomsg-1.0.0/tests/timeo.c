/*
    Copyright (c) 2012 Martin Sustrik  All rights reserved.

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
#include "../src/utils/stopwatch.c"
#include "../src/mlog/mlog_init.h"

int main ()
{
    printf("i am here, _F-L-F:%s-%u-%s\n", __FUNCTION__, __LINE__,
        __FILE__);

    int rc;
    int s;
    int timeo;
    char buf [3];
    struct nn_stopwatch stopwatch;
    uint64_t elapsed;

    s = test_socket (AF_SP, NN_PAIR);
    mlog_byfunc("s:%u", s);
    timeo = 100;
    mlog_byfunc("timeo:%u", timeo);
    rc = nn_setsockopt (s, NN_SOL_SOCKET, NN_RCVTIMEO, &timeo, sizeof (timeo));
    errno_assert (rc == 0);
    nn_stopwatch_init (&stopwatch);
    mlog_msgbyfunc(&stopwatch,sizeof(stopwatch),"stop watch");
    rc = nn_recv (s, buf, sizeof (buf), 0);
    elapsed = nn_stopwatch_term (&stopwatch);
    errno_assert (rc < 0 && nn_errno () == ETIMEDOUT);
    time_assert (elapsed, 100000);

    timeo = 100;
    rc = nn_setsockopt (s, NN_SOL_SOCKET, NN_SNDTIMEO, &timeo, sizeof (timeo));
    errno_assert (rc == 0);
    nn_stopwatch_init (&stopwatch);
    rc = nn_send (s, "ABC", 3, 0);
    elapsed = nn_stopwatch_term (&stopwatch);
    errno_assert (rc < 0 && nn_errno () == ETIMEDOUT);
    time_assert (elapsed, 100000);

    test_close (s);
    

    showmlogkeys();
    showmlogall();
    int color = 100;
    printf("\033[7m%dHello, world.\033[0m", color);
    const char * filewithpath = "./mlog.all.txt";
    savemlog2fileall(filewithpath);
    showmlogkeys();
    return 0;
}

