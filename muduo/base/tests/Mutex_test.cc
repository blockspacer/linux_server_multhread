/*
 * Mutex_test.cc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <muduo/base/Timestamp.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <stdio.h>

using namespace muduo;
using namespace std;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 10*1000*1000;

void threadFunc()
{
    for (int i=0; i<kCount; ++i) {
        MutexLockGuard lock(g_mutex);
        g_vec.push_back(i);
    }
}

int main()
{
    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads*kCount);

    Timestamp start(Timestamp::now());
    for (int i=0; i<kCount; ++i) {
        g_vec.push_back(i);
    }
    printf("single thread without lock %f\n", 
            timeDifference(Timestamp::now(), start));

    for (int nthreads=1; nthreads < kMaxThreads; ++nthreads) {
        boost::ptr_vector<Thread> threads;
        g_vec.clear();
        start = Timestamp::now();
        for (int i=0; i<nthreads; ++i) {
            threads.push_back(new Thread(&threadFunc));
            threads.back().start();
        }

        for (int i=0; i<nthreads; ++i) {
            threads[i].join();
        }
        printf("%d thread(s) with lock %f\n", nthreads, 
                timeDifference(Timestamp::now(), start));
    }
}