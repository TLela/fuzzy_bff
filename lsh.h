#ifndef LSH_H
#define LSH_H


template <typename InputType, typename ItemType>
class LSH {
public:
    LSH() {
    }
    virtual ~LSH() {
    }
    virtual ItemType hashed(const InputType& input) {
        return input;
    }
};

#endif // LSH_H
