#include <iostream>
#include <mutex>
#include <string>
#include <sstream>
#include <stdexcept>
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
        std::shared_lock guard0(mu0);
        std::shared_lock guard1(mu1);
        std::shared_lock guard2(mu2);
        std::ostringstream os;
        os << x1 << ", " << x2 << ", " << x3 << std::endl;
        return os.str();
    }
};

struct descriptionAction {
    std::string nameAction;
    int field, value;
};
std::vector<descriptionAction> makeVector(const std::string& path) {
    std::ifstream file(path);
    std::vector<descriptionAction> act;
    std::string name;
    while (file >> name) {
        if (name == "read") {
            int fieldIndex;
            file >> fieldIndex;
            if (fieldIndex == 0 || fieldIndex == 1 || fieldIndex == 2) {
                act.push_back({"read", fieldIndex, 0});
            }
        }
        if (name == "write") {
            int fieldIndex, value;
            file >> fieldIndex >> value;
            if (fieldIndex == 0 || fieldIndex == 1 || fieldIndex == 2) {
                act.push_back({"write", fieldIndex, 1});
            }
            if (name == "string") {
                act.push_back({"string", -1, 0});
            }
        }
    }
    return act;
}