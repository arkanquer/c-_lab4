#include <iostream>
#include <string>
#include <sstream>
#include <shared_mutex>
#include <fstream>
#include <random>
#include <thread>

class ThreeFields {
    mutable std::shared_mutex mu0, mu1, mu2;
    int x1, x2, x3;
public:
    ThreeFields(int a=0, int b=0, int c=0) : x1(a), x2(b), x3(c) {}

    int  get0() const {
        std::shared_lock guard(mu0);
        return x1;
    }
    int  get1() const {
        std::shared_lock guard(mu1);
        return x2;
    }
    int  get2() const {
        std::shared_lock guard(mu2);
        return x3;
    }

    void set0(int value) {
        std::unique_lock guard(mu0);
        x1 = value;
    }
    void set1(int value) {
        std::unique_lock guard(mu1);
        x2 = value;
    }
    void set2(int value) {
        std::unique_lock guard(mu2);
        x3 = value;
    }

    operator std::string() const {
        std::shared_lock guard0(mu0), guard1(mu1), guard2(mu2);
        std::ostringstream os;
        os << x1 << ", " << x2 << ", " << x3 << std::endl;
        return os.str();
    }
};

void makeFile(const std::string& filepath, std::size_t totalActs,
                   std::initializer_list<double> percents,
                   uint64_t seed)
{
    std::ofstream out(filepath);
    std::mt19937 gen(static_cast<uint32_t>(seed));
    std::discrete_distribution choice(percents);
    int value = 1;
    for (std::size_t i = 0; i < totalActs; ++i) {
        int k = choice(gen);
        if (k == 0) {
            out << "read 0\n";
        }
        else if (k == 1) {
            out << "write 0 " << value << '\n';
        }
        else if (k == 2) {
            out << "read 1\n";
        }
        else if (k == 3) {
            out << "write 1 " << value << '\n';
        }
        else if (k == 4) {
            out << "read 2\n";
        }
        else if (k == 5) {
            out << "write 2 " << value << '\n';
        }
        else {
            out << "string\n";
        }
    }
}
void process(ThreeFields& work, const std::string& text) {
    std::istringstream iss(text);
    std::string name;
    while (iss >> name) {
        if (name == "read") {
            int fieldIdx;
            iss >> fieldIdx;
            if(fieldIdx == 0) {
                work.get0();
            }
            if (fieldIdx == 1) {
                work.get1();
            }
            if (fieldIdx == 2) {
                work.get2();
            }
        }
        if (name == "write") {
            int fieldIdx, value;
            iss >> fieldIdx >> value;
            if (fieldIdx == 0)  {
                work.set0(value);
            }
            if (fieldIdx == 1) {
                work.set1(value);
            }
            if (fieldIdx == 2) {
                work.set2(value);
            }
        }
        if (name == "string") {
            std::string str = work;
        }
    }
}
template<class F>
long long measureTime(F&& func) {
    auto time0 = std::chrono::steady_clock::now();
    func();
    auto time1 = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(time1 - time0).count();
}
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::ostringstream os;
    os << file.rdbuf();
    return os.str();
}
long long runThreads(int threads, const std::string* txt1,
                     const std::string* txt2,const std::string* txt3)
{
    ThreeFields work(0,0,0);
    return measureTime([&]{
        if (threads == 1) {
            std::thread t1([&] {
                process(work, *txt1);
            });
            t1.join();
        }
        else if (threads == 2) {
            std::thread t1([&] {
                process(work, *txt1);
            });
            std::thread t2([&] {
                process(work, *txt2);
            });
            t1.join(); t2.join();
        }
        else if (threads == 3) {
            std::thread t1([&]{ process(work, *txt1); });
            std::thread t2([&]{ process(work, *txt2); });
            std::thread t3([&]{ process(work, *txt3); });
            t1.join(); t2.join(); t3.join();
        }
    }
    );
}

void measureThreads(const std::string& name,
                   std::initializer_list<double> percents,
                   std::size_t acts, uint64_t seed_base = 1234567)
{
    makeFile(name + ".thread1.txt", acts, percents, seed_base + 11);
    std::string exp1_1 = readFile(name + ".thread1.txt");
    long long time1 = runThreads(1, &exp1_1, nullptr, nullptr);

    makeFile(name + ".thread1.txt", acts, percents, seed_base + 21);
    makeFile(name + ".thread2.txt", acts, percents, seed_base + 22);
    std::string exp2_1 = readFile(name + ".thread1.txt");
    std::string exp2_2 = readFile(name + ".thread2.txt");
    long long time2 = runThreads (2, &exp2_1, &exp2_2, nullptr);

    makeFile(name + ".thread1.txt", acts, percents, seed_base + 31);
    makeFile(name + ".thread2.txt", acts, percents, seed_base + 32);
    makeFile(name + ".thread3.txt", acts, percents, seed_base + 33);
    std::string exp3_1 = readFile(name + ".thread1.txt");
    std::string exp3_2 = readFile(name + ".thread2.txt");
    std::string exp3_3 = readFile(name + ".thread3.txt");
    long long time3 = runThreads(3, &exp3_1, &exp3_2, &exp3_3);
    std::cout << name << " " << time1 << "ms " << time2 << "ms " << time3 << "ms\n";
}

int main() {
    const std::size_t acts = 500'000;
    measureThreads("Var6Condition", {20, 5, 20, 5, 20, 5, 25}, acts);
    measureThreads("EqualCondition", {1, 1, 1, 1, 1, 1, 1}, acts);
    measureThreads("AnotherCondition", {1,40, 1,40, 1,7, 10}, acts);
    return 0;
}