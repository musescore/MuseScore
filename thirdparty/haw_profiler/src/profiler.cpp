#include "profiler.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <errno.h>
#include <stdio.h>

using namespace haw::profiler;

Profiler::Options Profiler::m_options;

constexpr int MAIN_THREAD_INDEX(0);

Profiler* Profiler::instance()
{
    static Profiler p;
    return &p;
}

Profiler::Profiler()
{
    setup(Options(), new Printer());
}

Profiler::~Profiler()
{
    delete m_printer;
}

void Profiler::setup(const Options& opt, Printer* printer)
{
    m_options = opt;
    m_options.funcsMaxThreadCount = m_options.funcsMaxThreadCount < 1 ? 1 : m_options.funcsMaxThreadCount;

    //! Func timers
    m_funcs.timers.resize(m_options.funcsMaxThreadCount);
    std::fill(m_funcs.timers.begin(), m_funcs.timers.end(), FuncTimers());

    m_funcs.threads.resize(m_options.funcsMaxThreadCount);
    std::fill(m_funcs.threads.begin(), m_funcs.threads.end(), std::thread::id());
    m_funcs.threads[MAIN_THREAD_INDEX] = std::this_thread::get_id();

    if (printer) {
        delete m_printer;
        m_printer = printer;
    }
}

const Profiler::Options& Profiler::options()
{
    return m_options;
}

Profiler::Printer* Profiler::printer() const
{
    return m_printer;
}

void Profiler::stepTime(const std::string& tag, const std::string& info, bool isRestart)
{
    std::lock_guard<std::mutex> lock(m_steps.mutex);

    StepTimer* timer{ nullptr };
    StepTimers::const_iterator it = m_steps.timers.find(tag);
    if (it == m_steps.timers.end()) {
        timer = new StepTimer();
        timer->start();
        m_steps.timers.insert({ tag, timer });
    } else {
        timer = it->second;
    }

    if (isRestart) {
        timer->start();
    }

    printer()->printStep(tag, timer->beginMs(), timer->stepMs(), info);

    timer->nextStep();
}

Profiler::FuncTimer* Profiler::beginFunc(const std::string& func)
{
    std::thread::id th = std::this_thread::get_id();
    int idx = m_funcs.threadIndex(th);
    if (idx == -1) {
        idx = m_funcs.addThread(th);
        if (idx == -1) {
            return nullptr;
        }
    }
    size_t index = static_cast<size_t>(idx);

    FuncTimer* timer{ nullptr };
    FuncTimers& fts = m_funcs.timers[index];
    FuncTimers::const_iterator it = fts.find(&func);
    if (it == fts.end()) {
        timer = new FuncTimer(func);
        fts.insert({ &func, timer });
    } else {
        timer = it->second;
        if (timer->timer.isValid()) {
            //! NOTE Recursion detected, measure only first call
            //printer()->printDebug(std::string("Recursion detected, measure only first call, func: ") + func);
            return timer;
        }
    }

    timer->timer.start();

    if (m_options.funcsTraceEnabled) {
        ++m_stackCounter;
        printer()->printTraceBegin(func, m_stackCounter);
    }

    return timer;
}

void Profiler::endFunc(FuncTimer* timer, const std::string& func)
{
    if (timer) {
        if (timer->timer.isValid()) { //! NOTE If not valid then it is probably a recursive call
            double calltimeMs = timer->timer.mlsecsElapsed();

            timer->callcount++;
            timer->sumtimeMs += calltimeMs;
            timer->timer.invalidate();

            if (m_options.funcsTraceEnabled) {
                printer()->printTraceEnd(func, calltimeMs, timer->callcount, timer->sumtimeMs, m_stackCounter);
                --m_stackCounter;
            }
        }
    }
}

const std::string& Profiler::staticInfo(const std::string& info)
{
    auto found = m_funcs.staticInfo.find(info);
    if (found != m_funcs.staticInfo.end()) {
        return *found;
    }

    auto ins = m_funcs.staticInfo.insert(info);
    return *ins.first;
}

void Profiler::clear()
{
    {
        std::lock_guard<std::mutex> lock(m_funcs.mutex);

        std::thread::id empty;
        for (size_t i = 0; i < m_funcs.threads.size(); ++i) {
            FuncTimers timers = m_funcs.timers[i];
            for (auto ft : timers) {
                delete ft.second;
            }
            m_funcs.timers[i] = FuncTimers();
            m_funcs.threads[i] = empty;
        }
        m_funcs.threads[MAIN_THREAD_INDEX] = std::this_thread::get_id();
    }
    {
        std::lock_guard<std::mutex> lock(m_funcs.mutex);
        for (auto st : m_steps.timers) {
            delete st.second;
        }
        m_steps.timers.clear();
    }
}

Profiler::Data Profiler::threadsData(Data::Mode mode) const
{
    Data data;
    data.mainThread = m_funcs.threads[MAIN_THREAD_INDEX];

    std::thread::id empty;
    for (size_t i = 0; i < m_funcs.threads.size(); ++i) {
        std::thread::id th = m_funcs.threads[i];
        if (th == empty) {
            break;
        }

        if (i == MAIN_THREAD_INDEX) {
            if (mode == Data::OnlyOther) {
                continue;
            }
        } else {
            if (mode == Data::OnlyMain) {
                continue;
            }
        }

        Data::Thread thdata;
        thdata.thread = th;

        const FuncTimers& timers = m_funcs.timers[i];
        for (auto it : timers) {
            const FuncTimer* ft = it.second;
            Data::Func f(
                ft->func,
                ft->callcount,
                ft->sumtimeMs
                );

            thdata.funcs[f.func] = f;
        }

        data.threads[thdata.thread] = thdata;

        if (i == MAIN_THREAD_INDEX) {
            if (mode == Data::OnlyMain) {
                break;
            }
        }
    }

    return data;
}

std::string Profiler::threadsDataString(Data::Mode mode) const
{
    Profiler::Data data = threadsData(mode);
    return printer()->formatData(data, mode, m_options.dataTopCount);
}

void Profiler::printThreadsData(Data::Mode mode) const
{
    Profiler::Data data = threadsData(mode);
    printer()->printData(data, mode, m_options.dataTopCount);
}

void Profiler::print(const std::string& str)
{
    Profiler::instance()->printer()->printInfo(str);
}

bool Profiler::save(const std::string& filePath)
{
    std::string content = threadsDataString();
    bool ok = save_file(filePath, content);
    return ok;
}

bool Profiler::save_file(const std::string& path, const std::string& content)
{
    FILE* pFile = fopen(path.c_str(), "w");
    if (!pFile) {
        return false;
    }
    const char* data = content.c_str();
    size_t count = fwrite(data, sizeof(char), content.size(), pFile);
    fclose(pFile);

    return count > 0;
}

std::string FuncMarker::formatSig(const std::string& sig)
{
    static const std::string Coln("::");
    static const std::string Spc(" ");
    static const std::string ArgBeg("(");

    std::size_t endFunc = sig.find_first_of(ArgBeg);
    if (endFunc == std::string::npos) {
        return sig;
    }

    std::size_t beginFunc = sig.find_last_of(Coln, endFunc);
    if (beginFunc == std::string::npos) {
        return sig;
    }

    std::size_t beginClassColon = sig.find_last_of(Coln, beginFunc - 2);
    std::size_t beginClassSpace = sig.find_last_of(Spc, beginFunc - 2);

    std::size_t beginClass = std::string::npos;
    if (beginClassColon == std::string::npos) {
        beginClass = beginClassSpace;
    } else if (beginClassSpace == std::string::npos) {
        beginClass = beginClassColon;
    } else {
        beginClass = std::max(beginClassColon, beginClassSpace);
    }

    if (beginClass == std::string::npos) {
        beginClass = beginFunc;
    } else {
        beginClass += 1;
    }

    std::string str = sig.substr(beginClass, (endFunc - beginClass));
    return str;
}

double Profiler::StepTimer::beginMs() const
{
    return beginTime.mlsecsElapsed();
}

double Profiler::StepTimer::stepMs() const
{
    return stepTime.mlsecsElapsed();
}

void Profiler::StepTimer::start()
{
    beginTime.start();
    stepTime.start();
}

void Profiler::StepTimer::restart()
{
    beginTime.restart();
    stepTime.restart();
}

void Profiler::StepTimer::nextStep()
{
    stepTime.restart();
}

int Profiler::FuncsData::threadIndex(std::thread::id th) const
{
    std::thread::id empty;
    size_t count = this->threads.size();
    for (size_t i = 0; i < count; ++i) {
        std::thread::id thi = this->threads[i];
        if (thi == th) {
            return static_cast<int>(i);
        }

        if (thi == empty) {
            return -1;
        }
    }
    return -1;
}

int Profiler::FuncsData::addThread(std::thread::id th)
{
    std::lock_guard<std::mutex> lock(this->mutex);

    int index = threadIndex(th);
    if (index > -1) {
        return index;
    }

    size_t count = this->threads.size();
    std::thread::id empty;
    for (size_t i = 0; i < count; ++i) {
        std::thread::id thi = this->threads[i];
        if (thi == empty) {
            this->threads[i] = th;
            return static_cast<int>(i);
        }
    }
    return -1;
}

using mclock = std::chrono::high_resolution_clock;

void Profiler::ElapsedTimer::start()
{
    _start = mclock::now();
}

void Profiler::ElapsedTimer::restart()
{
    start();
}

double Profiler::ElapsedTimer::mlsecsElapsed() const
{
    auto end = mclock::now();
    std::chrono::duration<double, std::milli> elapsed = end - _start;

    return elapsed.count();
}

void Profiler::ElapsedTimer::invalidate()
{
    _start = mclock::time_point();
}

bool Profiler::ElapsedTimer::isValid() const
{
    const mclock::duration since_epoch = _start.time_since_epoch();
    return since_epoch.count() > 0;
}

Profiler::Printer::~Printer()
{}

void Profiler::Printer::printDebug(const std::string& str)
{
    std::cout << str << '\n';
}

void Profiler::Printer::printInfo(const std::string& str)
{
    std::cout << str << '\n';
}

static std::string formatDouble(double val, size_t prec)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(prec) << val;
    return ss.str();
}

void Profiler::Printer::printStep(const std::string& tag, double beginMs, double stepMs, const std::string& info)
{
    static const std::string COLN(" : ");
    static const std::string SEP("/");
    static const std::string MS(" ms: ");

    std::string str;
    str.reserve(100);

    str
    .append(tag)
    .append(COLN)
    .append(formatDouble(beginMs, 3))
    .append(SEP)
    .append(formatDouble(stepMs, 3))
    .append(MS)
    .append(info);

    printDebug(str);
}

void Profiler::Printer::printTraceBegin(const std::string& func, size_t stackCounter)
{
    static const std::string BEGIN("-> ");

    std::string indent;
    indent.resize(stackCounter, ' ');
    printDebug(indent + BEGIN + func);
}

void Profiler::Printer::printTraceEnd(const std::string& func, double calltimeMs, long callcount, double sumtimeMs, size_t stackCounter)
{
    static const std::string END("<- ");
    static const std::string CALLTIME("  calltime: ");
    static const std::string CALLCOUNT("  callcount: ");
    static const std::string SUMTIME("  sumtime: ");

    std::string indent;
    indent.resize(stackCounter, ' ');

    //if (calltimeMs > 10 || callcount%100 == 1.) { //! NOTE Anti spam
    std::string str;
    str.reserve(100);
    str
    .append(indent)
    .append(END)
    .append(func)
    .append(CALLTIME).append(formatDouble(calltimeMs, 3))
    .append(CALLCOUNT).append(std::to_string(callcount))
    .append(SUMTIME).append(formatDouble(sumtimeMs, 3));

    printDebug(str);
    //}
}

void Profiler::Printer::printData(const Data& data, Data::Mode mode, int maxcount)
{
    printInfo(formatData(data, mode, maxcount));
}

std::string Profiler::Printer::formatData(const Data& data, Data::Mode mode, int maxcount) const
{
    struct IsLessBySum {
        bool operator()(const Profiler::Data::Func& f, const Profiler::Data::Func& s) const
        {
            return f.sumtimeMs > s.sumtimeMs;
        }
    };

    std::stringstream stream;
    stream << "\n\n";

    using Funcs = std::unordered_map<std::string, Data::Func>;
    using Threads = std::unordered_map<std::thread::id, Data::Thread>;

    if (mode == Data::OnlyMain || mode == Data::All) {
        std::list<Data::Func> list;
        const Funcs& funcs = data.threads.at(data.mainThread).funcs;
        Funcs::const_iterator it = funcs.cbegin(), end = funcs.cend();
        for (; it != end; ++it) {
            list.push_back(it->second);
        }

        list.sort(IsLessBySum());

        std::string info = "Main thread. Top "
                           + std::to_string(maxcount)
                           + " by sum time (total count: "
                           + std::to_string(list.size())
                           + ")";

        funcsToStream(stream, info, list, maxcount);
    }

    if (mode == Data::OnlyOther || mode == Data::All) {
        std::list<Data::Func> list;
        Threads::const_iterator it = data.threads.cbegin(), end = data.threads.cend();
        for (; it != end; ++it) {
            if (it->first == data.mainThread) {
                continue;
            }

            const Funcs& funcs = it->second.funcs;
            Funcs::const_iterator it = funcs.cbegin(), end = funcs.cend();
            for (; it != end; ++it) {
                list.push_back(it->second);
            }
        }

        list.sort(IsLessBySum());

        std::string info = "Other threads. Top "
                           + std::to_string(maxcount)
                           + " by sum time (total count: "
                           + std::to_string(list.size())
                           + ")";

        funcsToStream(stream, info, list, maxcount);
    }

    return stream.str();
}

void Profiler::Printer::funcsToStream(std::stringstream& stream,
                                      const std::string& title,
                                      const std::list<Data::Func>& funcs,
                                      int count) const
{
    auto leftJustified = [](const std::string& val, size_t width) -> std::string
                         {
                             std::string str;
                             str.resize(width, ' ');
                             size_t lenght = width < val.size() ? width : val.size();
                             for (size_t i = 0; i < lenght; ++i) {
                                 str[i] = val[i];
                             }
                             return str;
                         };

    #define FORMAT(str, width) leftJustified(str, width)
    #define TITLE(str) FORMAT(std::string(str), 18)
    #define VALUE(val, unit) FORMAT(std::to_string(val) + unit, 18)
    #define VALUE_D(val, unit) FORMAT(formatDouble(val, 3) + unit, 18)

    stream << title << "\n";
    stream << FORMAT("Function", 60) << TITLE("Call time") << TITLE("Call count") << TITLE("Sum time") << "\n";

    int i = 0;
    for (const Data::Func& f : funcs) {
        stream << FORMAT(f.func, 60)
               << VALUE_D(f.callcount ? (f.sumtimeMs / static_cast<double>(f.callcount)) : 0, " ms")
               << VALUE(f.callcount, "")
               << VALUE_D(f.sumtimeMs, " ms")
               << "\n";
        ++i;
        if (i >= count) {
            break;
        }
    }

    #undef FORMAT
    #undef TITLE
    #undef VALUE
    #undef VALUE_D

    stream << "\n\n";
}
