/*
 * BlockingQueue_bench.cc
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

#include <muduo/base/BlockingQueue.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <map>
#include <string>
#include <stdio.h>

class Bench
{
public:
    Bench(int numThreads)
        : latch_(numThreads),
          threads_()
    {
        for (int i=0; i<numThreads; ++i) {
            char name[32] = {0};
            snprintf(name, sizeof name, "work thread %d", i);
            threads_.push_back(new muduo::Thread(
                        boost::bind(&Bench::threadFunc, this),
                        muduo::string(name)));
        }

        for_each(threads_.begin(), threads_.end(),
                boost::bind(&muduo::Thread::start, _1));
    }

    void run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.wait();
        printf("all threads started\n");
        for (int i=0; i<times; ++i) {
            muduo::Timestamp now(muduo::Timestamp::now());
            queue_.put(now);
            usleep(100);
        }
    }

    void joinAll()
    {
        for (size_t i=0; i<threads_.size(); ++i) {
            queue_.put(muduo::Timestamp::invalid());
        }
        for_each(threads_.begin(), threads_.end(),
                boost::bind(&muduo::Thread::join, _1));
    }

private:
    void threadFunc()
    {
        printf("tid=%d, %s started\n", muduo::CurrentThread::tid(),
                muduo::CurrentThread::name());

        std::map<int, int> delays;
        latch_.countDown();
        
        bool running = true;
        while (running) {
            muduo::Timestamp t(queue_.take());
            muduo::Timestamp now(muduo::Timestamp::now());
            if (t.valid()) {
                int delay = static_cast<int>(timeDifference(now, t)*1000000);
                ++delays[delay];
            }
            running = t.valid();
        }

        printf("tid=%d, %s stopped\n", muduo::CurrentThread::tid(),
                muduo::CurrentThread::name());

        for (std::map<int, int>::iterator ibeg = delays.begin();
                ibeg != delays.end(); ++ibeg) {
            printf("tid=%d, delay = %d, count = %d\n",
                    muduo::CurrentThread::tid(),
                    ibeg->first, ibeg->second);
        }

    }

private:
    muduo::BlockingQueue<muduo::Timestamp> queue_;
    muduo::CountDownLatch latch_;
    boost::ptr_vector<muduo::Thread> threads_;
};

int main()
{
    Bench b(5);
    b.run(10000);
    b.joinAll();
}
