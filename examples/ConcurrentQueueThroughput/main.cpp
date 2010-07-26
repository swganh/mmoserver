#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <omp.h>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include "Utils/ConcurrentQueue.h"

const int SMALL_STRING_COUNT = 10;
const int LARGE_STRING_COUNT = 50;

struct TestObject {
    TestObject() {}
    TestObject(int count) {
        std::ostringstream stream;
        for (int i = 0; i < count; ++i) {
            stream << "This is a string that is long enough for this test, or at least I hope so: " << i;
            strings.push_back(stream.str());
            stream.clear();
        }
    }

    std::vector<std::string> strings;
};

/*
// single-threaded concurrent queue - small objects
int main() {
    ConcurrentQueue<TestObject> the_queue;
    
    // Starting the time measurement
    double start = omp_get_wtime();
    int counter = 0;

    TestObject small_test_object(SMALL_STRING_COUNT);

    // single-threaded    
    for (int i = 0; i < 100000; ++i) {
        the_queue.push(small_test_object);
    }
    
    TestObject test_object;
    while (the_queue.pop(test_object)) {
        ++counter;
    }

    // Measuring the elapsed time
    double end = omp_get_wtime();

    std::cout << "Single-threaded small objects took (" << (end - start) << ") seconds to produce and consume " << counter << " objects." << std::endl;

    return 0;
}
*/


// threaded concurrent queue - small objects
int main() {
    ConcurrentQueue<TestObject> the_queue;
    
    // Starting the time measurement
    double start = omp_get_wtime();
    int counter = 0;

    TestObject small_test_object(SMALL_STRING_COUNT);
    TestObject large_test_object(SMALL_STRING_COUNT);

    // threaded
    boost::thread thread1([&the_queue, &small_test_object] {
        for (int i = 0; i < 50000; ++i) {
            the_queue.push(small_test_object);
        }
    });
    
    boost::thread thread2([&the_queue, &small_test_object] {
        for (int i = 0; i < 50000; ++i) {
            the_queue.push(small_test_object);
        }
    });

    boost::thread thread5([&the_queue, &small_test_object, &counter] {
        TestObject test_object;
        while (the_queue.pop(test_object)) {
            ++counter;
        }
    });
    
    thread1.join();
    thread2.join();
    thread5.join();

    // Measuring the elapsed time
    double end = omp_get_wtime();

    std::cout << "Threaded small objects took (" << (end - start) << ") seconds to produce and consume " << counter << " objects." << std::endl;

    return 0;
}

/* // std::queue version - small objects
int main() {
    std::queue<TestObject> the_queue;
    
    // Starting the time measurement
    double start = omp_get_wtime();
    int counter = 0;

    TestObject small_test_object(SMALL_STRING_COUNT);

    // threaded    
    for (int i = 0; i < 100000; ++i) {
        the_queue.push(small_test_object);
    }
    
    TestObject test_object;
    while (!the_queue.empty()) {
        the_queue.pop();
        ++counter;
    }

    // Measuring the elapsed time
    double end = omp_get_wtime();

    std::cout << "Single-threaded std::queue small objects took (" << (end - start) << ") seconds to produce and consume " << counter << " objects." << std::endl;

    return 0;
}
*/

