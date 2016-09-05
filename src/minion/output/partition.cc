#include "partition.h"

#include <algorithm>

namespace baidu {
namespace shuttle {

class KeyFieldBasedPartitioner : public Partitioner {
public:
    KeyFieldBasedPartitioner(const NodeConfig& node, int dest_num);
    virtual ~KeyFieldBasedPartitioner() { }

    int Calc(const std::string& line, std::string* key) const;
    int Calc(const std::string& key) const;
private:
    int num_key_fields_;
    int num_partition_fields_;
    int dest_num_;
    std::string separator_;
};

class IntHashPartitioner : public Partitioner {
public:
    IntHashPartitioner(const NodeConfig& node, int dest_num);
    virtual ~IntHashPartitioner() { }

    int Calc(const std::string& line, std::string* key) const;
    int Calc(const std::string& key) const;
private:
    int dest_num_;
    std::string separator_;
};

Partitioner* Partitioner::Get(Partition partitioner,
        const NodeConfig& node, int dest_num) {
    switch(partitioner) {
    case kKeyFieldBasedPartitioner:
        return new KeyFieldBasedPartitioner(node, dest_num);
    case kIntHashPartitioner:
        return new IntHashPartitioner(node, dest_num);
    }
    return NULL;
}

int Partitioner::HashCode(const std::string& str) const {
    int h = 1;
    if (str.empty()) {
        return 0;
    }
    for (size_t i = 0;  i < str.size(); i++) {
        h = 31 * h + str[i];
    }
    return h & 0x7FFFFFFF;
}

KeyFieldBasedPartitioner::KeyFieldBasedPartitioner(const NodeConfig& node, int dest_num) :
        num_key_fields_(0),
        num_partition_fields_(0),
        dest_num_(dest_num) {
    num_key_fields_ = node.key_fields_num();
    num_partition_fields_ = node.partition_fields_num();
    separator_ = node.key_separator();

    // Default values
    if (num_key_fields_ == 0) {
        num_key_fields_ = 1;
    }
    if (num_partition_fields_ == 0) {
        num_partition_fields_ = 1;
    }
    if (separator_.empty()) {
        separator_ = "\t";
    }
}

int KeyFieldBasedPartitioner::Calc(const std::string& line, std::string* key) const {
    if (key == NULL) {
        return -1;
    }
    // Parse key from record
    const char* head = line.data();
    const char *p1 = head;
    const char *p2 = head;
    const char* end = head + line.size();
    int N = std::max(num_key_fields_, num_partition_fields_);
    for (int i = 0; i < N; i++) {
        if (i < num_key_fields_) {
            if (p1 >= end) {
                break;
            }
            p1 += (strcspn(p1, separator_.c_str()) + 1);
        }
        if (i < num_partition_fields_) {
            if (p2 >= end) {
                break;
            }
            p2 += (strcspn(p2, separator_.c_str()) + 1);
        }
    }
    if (p1 == head) {
        p1 = head + 1;
    }
    if (p2 == head) {
        p2 = head + 1;
    }
    key->assign(head, p1 - 1);
    std::string partition_key(head, p2 - 1);
    return HashCode(partition_key) % dest_num_; 
}

int KeyFieldBasedPartitioner::Calc(const std::string& key) const {
    return HashCode(key) % dest_num_;
}

IntHashPartitioner::IntHashPartitioner(const NodeConfig& node, int dest_num) :
        dest_num_(dest_num) {
    separator_ = node.key_separator();
    if (separator_.empty()) {
        separator_ = "\t";
    }
}

int IntHashPartitioner::Calc(const std::string& line, std::string* key) const {
    // A record is organized as "[int][space]...[key][separator][value]"
    //   e.g.: "123  key_0\tvalue0"
    size_t space_pos = line.find(" ");
    int hash_code;
    if (space_pos != std::string::npos) {
        // Use space to get the int-hash part
        hash_code = atoi(line.substr(0, space_pos).c_str());
        const char *p = line.data() + space_pos + 1;
        int key_span = strcspn(p, separator_.c_str());
        key->assign(p, key_span);
    } else {
        // No space means ordinary key hash
        const char *p = line.data();
        int key_span = strcspn(p, separator_.c_str());
        key->assign(p, key_span);
        hash_code = HashCode(*key);
    }
    return hash_code % dest_num_;
}

int IntHashPartitioner::Calc(const std::string& key) const {
    size_t space_pos = key.find(" ");
    int hash_code;
    if (space_pos != std::string::npos) {
        hash_code = atoi(key.substr(0, space_pos).c_str());
    } else {
        hash_code = HashCode(key);
    }
    return hash_code % dest_num_;
}

}
}

