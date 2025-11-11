#include <iostream>
#include <string>
#include <sstream>
#include <shared_mutex>
#include <vector>
#include <fstream>

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

enum class actName {
    Read, Write, String
};
struct action {
    actName name;
    int field = -1;
    int value = 0;
};

std::vector<action> makeVector(const std::string& path) {
    std::ifstream file(path);
    std::vector<action> acts;
    std::string act;
    while (file >> act) {
        if (act == "read") {
            int fieldIndex;
            file >> fieldIndex;
            acts.push_back({actName::Read, fieldIndex, 0});
        }
        else if (act == "write") {
            int fieldIndex, value;
            file >> fieldIndex >> value;
            acts.push_back({actName::Write, fieldIndex, value});
        }
        else if (act == "string") {
            acts.push_back({actName::String, -1, 0});
        }
    }
    return acts;
}

void process(ThreeFields& work, const std::vector<action>& acts) {
    for (const auto& act : acts) {
        if (act.name == actName::Read) {
            if(act.field == 0) {
                work.get0();
            }
            if (act.field == 1) {
                work.get1();
            }
            if (act.field == 2) {
                work.get2();
            }
        }
        if (act.name == actName::Write) {
            if (act.field == 0)  {
                work.set0(act.value);
            }
            if (act.field == 1) {
                work.set1(act.value);
            }
            if (act.field == 2) {
                work.set2(act.value);
            }
        }
        if (act.name == actName::String) {
        }
    }
}