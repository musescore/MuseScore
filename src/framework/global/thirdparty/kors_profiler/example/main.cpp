#include <iostream>
#include <thread>
#include <chrono>

#include "profiler.h"

class Example
{
public:
    Example() = default;

    static void th_func()
    {
        TRACEFUNC;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    void func1()
    {
        TRACEFUNC;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    void func2()
    {
        TRACEFUNC;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    void func3()
    {
        TRACEFUNC;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    void func4()
    {
        TRACEFUNC;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        STEP_TIME("mark1", "end body func4");

        func3();
    }

    void example()
    {
        std::thread th(th_func);

        TRACEFUNC;
        func1();

        BEGIN_STEP_TIME("mark1");

        {
            TRACEFUNC_C("call func2 10 times")
            for (int i = 0; i < 10; ++i) {
                func2();
            }
        }

        STEP_TIME("mark1", "end call func2 10 times");

        func3();
        STEP_TIME("mark1", "end func3");

        func4();

        th.join();
    }
};

int main(int argc, char* argv[])
{
    std::cout << "Hello World, I am Profiler" << std::endl;

    Example t;
    t.example();

    PROFILER_PRINT;

    /* Output:
        mark1 : 0.000/0.000 ms: Begin
        mark1 : 21.582/21.545 ms: end call func2 10 times
        mark1 : 31.699/10.100 ms: end func3
        mark1 : 41.816/10.098 ms: end body func4


        Main thread. Top 150 by sum time (total count: 6)
        Function                                                    Call time         Call count        Sum time
        Example::example                                            54.148 ms         1                 54.148 ms
        call func2 10 times                                         21.520 ms         1                 21.520 ms
        Example::func2                                              2.150 ms          10                21.500 ms
        Example::func4                                              20.202 ms         1                 20.202 ms
        Example::func3                                              10.090 ms         2                 20.180 ms
        Example::func1                                              2.213 ms          1                 2.213 ms


        Other threads. Top 150 by sum time (total count: 1)
        Function                                                    Call time         Call count        Sum time
        Example::th_func                                            2.212 ms          1                 2.212 ms

    */

    //! NOTE Custom setup

    // custom printer
    using namespace kors::profiler;
    struct MyPrinter : public Profiler::Printer
    {
        void printDebug(const std::string& str) override { std::clog << str << std::endl; }
        void printInfo(const std::string& str) override { std::clog << str << std::endl; }
    };

    // options
    Profiler::Options profOpt;
    profOpt.stepTimeEnabled = true;         // enable measure of steps, macros: BEGIN_STEP_TIME, STEP_TIME
    profOpt.funcsTimeEnabled = true;        // enable measure of functions, macros: TRACEFUNC, TRACEFUNC_C
    profOpt.funcsTraceEnabled = false;      // enable trace (output by func call), macros: TRACEFUNC, TRACEFUNC_C
    profOpt.funcsMaxThreadCount = 100;      // max treads count
    profOpt.statTopCount = 150;             // statistic top count

    Profiler* profiler = Profiler::instance();
    profiler->setup(profOpt, new MyPrinter());
}
