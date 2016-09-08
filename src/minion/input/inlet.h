#ifndef _BAIDU_SHUTTLE_INLET_H_
#define _BAIDU_SHUTTLE_INLET_H_
#include <string>

namespace baidu {
namespace shuttle {

class Inlet {
public:
    virtual int Flow() = 0;

    virtual ~Inlet() { }
};

class SourceInlet : public Inlet {
public:
    SourceInlet() : is_nline_(false) { }
    virtual ~SourceInlet();

    virtual int Flow();
private:
    std::string type_;
    std::string format_;
    std::string file_;
    std::string pipe_;
    bool is_nline_;
    int64_t offset_;
};

class ShuffleInlet : public Inlet {
public:
    ShuffleInlet();
    virtual ~ShuffleInlet();

    virtual int Flow();
private:
    bool PreMerge(const std::vector<std::string>& files, const std::string& output);
    bool FinalMerge(const std::vector<std::string>& files);
private:
    std::string pipe_;
    int no_;
};

}
}

#endif

