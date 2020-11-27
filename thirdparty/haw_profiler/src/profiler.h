#ifndef HAW_PROFILER_H
#define HAW_PROFILER_H

#include <string>
#include <list>
#include <vector>
#include <set>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <sstream>

#define HAW_PROFILER_ENABLED

#ifndef FUNC_INFO
#if defined(_MSC_VER)
    #define FUNC_INFO __FUNCSIG__
#else
    #define FUNC_INFO __PRETTY_FUNCTION__
#endif
#endif

#ifdef HAW_PROFILER_ENABLED

#ifndef TRACEFUNC
#define TRACEFUNC \
    static std::string __func_info(haw::profiler::FuncMarker::formatSig(FUNC_INFO)); \
    haw::profiler::FuncMarker __funcMarker(__func_info);
#endif

#ifndef TRACEFUNC_C
#define TRACEFUNC_C(info) \
    static std::string __func_info(info); \
    haw::profiler::FuncMarker __funcMarkerInfo(__func_info);
#endif

#ifndef BEGIN_STEP_TIME
#define BEGIN_STEP_TIME(tag) \
    if (haw::profiler::Profiler::options().stepTimeEnabled) \
    { haw::profiler::Profiler::instance()->stepTime(tag, std::string("Begin"), true); }
#endif

#ifndef STEP_TIME
#define STEP_TIME(tag, info) \
    if (haw::profiler::Profiler::options().stepTimeEnabled) \
    { haw::profiler::Profiler::instance()->stepTime(tag, info); }
#endif

#ifndef PROFILER_CLEAR
#define PROFILER_CLEAR haw::profiler::Profiler::instance()->clear();
#endif

#ifndef PROFILER_PRINT
#define PROFILER_PRINT haw::profiler::Profiler::instance()->printThreadsData();
#endif

#else

#define TRACEFUNC
#define TRACEFUNC_C(info)
#define BEGIN_STEP_TIME
#define STEP_TIME
#define PROFILER_CLEAR
#define PROFILER_PRINT

#endif

namespace haw::profiler {
class Profiler
{
public:

    static Profiler* instance()
    {
        static Profiler p;
        return &p;
    }

    struct Options {
        bool stepTimeEnabled{ true };
        bool funcsTimeEnabled{ true };
        bool funcsTraceEnabled{ false };
        size_t funcsMaxThreadCount{ 100 };
        int dataTopCount{ 150 };
        Options() {}
    };

    struct Data {
        enum Mode {
            All,
            OnlyMain,
            OnlyOther
        };

        struct Func {
            std::string func;
            long callcount{ 0 };
            double sumtimeMs{ 0. };
            Func() {}
            Func(const std::string& f, long cc, double st)
                : func(f), callcount(cc), sumtimeMs(st) {}
        };

        struct Thread {
            std::thread::id thread;
            std::unordered_map<std::string, Func> funcs;
        };

        std::thread::id mainThread;
        std::unordered_map<std::thread::id, Thread> threads;
    };

    struct Printer {
        virtual ~Printer();
        virtual void printDebug(const std::string& str);
        virtual void printInfo(const std::string& str);
        virtual void printStep(const std::string& tag, double beginMs, double stepMs, const std::string& info);
        virtual void printTraceBegin(const std::string& func, size_t stackCounter);
        virtual void printTraceEnd(const std::string& func, double calltimeMs, long callcount, double sumtimeMs, size_t stackCounter);
        virtual void printData(const Data& data, Data::Mode mode, int maxcount);
        virtual std::string formatData(const Data& data, Data::Mode mode, int maxcount) const;
        virtual void funcsToStream(std::stringstream& stream, const std::string& title, const std::list<Data::Func>& funcs,int count) const;
    };

    struct ElapsedTimer {
        void start();
        void restart();
        double mlsecsElapsed() const; //NOTE fractional milliseconds
        void invalidate();
        bool isValid() const;

    private:
        std::chrono::high_resolution_clock::time_point _start;
    };

    struct FuncTimer {
        const std::string& func;
        ElapsedTimer timer;
        long callcount;
        double sumtimeMs;
        explicit FuncTimer(const std::string& f)
            : func(f), callcount(0), sumtimeMs(0) {}
    };

    void setup(const Options& opt = Options(), Printer* printer = nullptr);

    static const Options& options();
    Printer* printer() const;

    void stepTime(const std::string& tag, const std::string& info, bool isRestart = false);

    FuncTimer* beginFunc(const std::string& func);
    void endFunc(FuncTimer* timer, const std::string& func);

    const std::string& staticInfo(const std::string& info); //! NOTE Saving string

    void clear();

    Data threadsData(Data::Mode mode = Data::All) const;

    std::string threadsDataString(Data::Mode mode = Data::All) const;
    void printThreadsData(Data::Mode mode = Data::All) const;

    static void print(const std::string& str);

    bool save(const std::string& filePath);

private:
    Profiler();
    ~Profiler();

    friend struct FuncMarker;

    static Options m_options;

    struct StepTimer {
        ElapsedTimer beginTime;
        ElapsedTimer stepTime;

        double beginMs() const;
        double stepMs() const;
        void start();
        void restart();
        void nextStep();
    };

    typedef std::unordered_map<std::string, StepTimer* > StepTimers;
    struct StepsData {
        std::mutex mutex;
        StepTimers timers;
    };

    typedef std::unordered_map<const std::string*, FuncTimer* > FuncTimers;
    struct FuncsData {
        std::mutex mutex;
        std::vector<std::thread::id> threads;
        std::vector<FuncTimers> timers;
        std::set<std::string> staticInfo;

        int threadIndex(std::thread::id th) const;
        int addThread(std::thread::id th);
    };

    bool save_file(const std::string& path, const std::string& content);

    Printer* m_printer{ nullptr };

    StepsData m_steps;
    FuncsData m_funcs;

    size_t m_stackCounter{ 0 };
};

struct FuncMarker
{
    explicit FuncMarker(const std::string& fn)
        : func(fn)
    {
        if (Profiler::m_options.funcsTimeEnabled) {
            timer = Profiler::instance()->beginFunc(fn);
        }
    }

    ~FuncMarker()
    {
        if (Profiler::m_options.funcsTimeEnabled) {
            Profiler::instance()->endFunc(timer, func);
        }
    }

    static std::string formatSig(const std::string& sig);

    Profiler::FuncTimer* timer{ nullptr };
    const std::string& func;
};
}

#endif // XTZ_PROFILER_H
