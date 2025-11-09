#include <mutex>
#include <string>
#include <sstream>
#include <stdexcept>
#include <shared_mutex>

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